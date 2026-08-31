/*
 * scedit -- sc_tcpclient.cpp
 *
 * POSIX sockets, nothing else: no library, no thread, no timer. The contract
 * and every measured fact about the engine's wire are in sc_tcpclient.hpp.
 */

#include "sc_tcpclient.hpp"

#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace scedit {

namespace {

std::string errnoText(int e)
{
	char buf[256];
	// The XSI form is the one glibc gives with _POSIX_C_SOURCE; strerror_r's
	// GNU form returns a pointer that may not be `buf`, so read the return.
	return std::string(::strerror_r(e, buf, sizeof(buf)));
}

//! Milliseconds since an arbitrary origin, monotonic -- used only for the
//! pollFor deadline, never for a measurement claim.
long nowMs()
{
	struct timespec ts;
	::clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

} // namespace

bool parseEndpoint(const std::string &spec, Endpoint &out, std::string &err)
{
	if (spec.empty()) {
		err = "an endpoint is `port` or `host:port`, e.g. `7805` or `dome:7805`; "
		      "nothing was given";
		return false;
	}
	Endpoint ep;
	std::string portText = spec;
	const std::size_t colon = spec.rfind(':');
	if (colon != std::string::npos) {
		ep.host = spec.substr(0, colon);
		portText = spec.substr(colon + 1);
		if (ep.host.empty()) {
			err = "`" + spec + "`: the host before the `:` is empty; write `port` for the "
			      "default host (127.0.0.1) or `host:port`";
			return false;
		}
	}
	if (portText.empty() || portText.find_first_not_of("0123456789") != std::string::npos) {
		err = "`" + spec + "`: `" + portText + "` is not a port number; an endpoint is "
		      "`port` or `host:port`, e.g. `7805` or `dome:7805`";
		return false;
	}
	const long p = std::strtol(portText.c_str(), nullptr, 10);
	if (p < 1 || p > 65535) {
		err = "`" + spec + "`: port " + portText + " is outside 1-65535";
		return false;
	}
	ep.port = (int)p;
	out = ep;
	return true;
}

FeedDiagnostic parseFeedDiagnostic(const std::string &line)
{
	FeedDiagnostic d;
	static const std::string PREFIX = "$DIAG|";
	if (line.compare(0, PREFIX.size(), PREFIX) != 0)
		return d;
	// Three splits, not four: the SUBJECT is a command line and may contain the
	// separator, so it takes whatever is left. The engine composes it in that
	// order for exactly this reason (INTENT 11.188).
	const std::size_t a = line.find('|', PREFIX.size());
	if (a == std::string::npos)
		return d;
	const std::size_t b = line.find('|', a + 1);
	if (b == std::string::npos)
		return d;
	d.ok = true;
	d.origin = line.substr(PREFIX.size(), a - PREFIX.size());
	d.message = line.substr(a + 1, b - a - 1);
	d.subject = line.substr(b + 1);
	return d;
}

TcpClient::~TcpClient()
{
	// No $LOGOFF from the destructor: a socket being torn down at exit cannot
	// wait for anything, and closing the connection already unsubscribes it
	// from BOTH channels (io.cpp's close clears clientBroadcastTab and
	// clientDiagTab). disconnect() is the polite path and the editor takes it.
	closeSocket();
}

void TcpClient::closeSocket()
{
	if (fd_ >= 0)
		::close(fd_);
	fd_ = -1;
	partial_.clear();
}

bool TcpClient::connect(const Endpoint &ep, std::string &err)
{
	if (connected())
		return true;
	closeSocket();
	endpoint_ = ep;

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	addrinfo *res = nullptr;
	const std::string port = std::to_string(ep.port);
	const int rc = ::getaddrinfo(ep.host.c_str(), port.c_str(), &hints, &res);
	if (rc != 0) {
		err = "cannot resolve " + ep.text() + ": " + ::gai_strerror(rc);
		last_error_ = err;
		state_ = LinkState::Failed;
		return false;
	}
	int sock = -1;
	std::string why;
	for (addrinfo *a = res; a != nullptr; a = a->ai_next) {
		sock = ::socket(a->ai_family, a->ai_socktype, a->ai_protocol);
		if (sock < 0) {
			why = errnoText(errno);
			continue;
		}
		if (::connect(sock, a->ai_addr, a->ai_addrlen) == 0)
			break;
		why = errnoText(errno);
		::close(sock);
		sock = -1;
	}
	::freeaddrinfo(res);
	if (sock < 0) {
		err = "cannot connect to " + ep.text() + ": " + why +
		      " \xe2\x80\x94 is spacecrafter running, and is `io:enable_tcp` true with "
		      "`io:tcp_port_in` = " + port + " in its config.ini?";
		last_error_ = err;
		state_ = LinkState::Failed;
		return false;
	}
	// Commands are short and the engine polls its socket set every millisecond;
	// Nagle would add up to 40 ms to a single-line write for nothing.
	int one = 1;
	::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	fd_ = sock;
	state_ = LinkState::Connected;
	last_error_.clear();
	bytes_in_ = 0;
	lines_sent_ = 0;
	diagnostics_in_ = 0;

	// The subscription, immediately: from here the feed carries every command
	// answer the engine produces, this client's own included (io.cpp:614).
	std::string serr;
	if (!writeAll("$LOGON\n", serr)) {
		err = "connected to " + ep.text() + " but could not subscribe: " + serr;
		last_error_ = err;
		state_ = LinkState::Failed;
		closeSocket();
		return false;
	}
	// ... and the DEDICATED diagnostic link, which is a second subscription and
	// not a second connection. A failure here is NOT fatal: on an engine older
	// than INTENT 11.188 this is simply an unrecognised command, and a working
	// connection must not be thrown away because the newer half of the protocol
	// is not there. What that costs is a quiet feed, which is what every engine
	// gave until now.
	if (!writeAll("$DIAGON\n", serr)) {
		note("connected to " + ep.text() + ", subscribed with $LOGON; $DIAGON could not be "
		     "sent (" + serr + ") - refusals will not appear on this feed");
		return true;
	}
	note("connected to " + ep.text() + ", subscribed with $LOGON and $DIAGON");
	return true;
}

void TcpClient::disconnect()
{
	if (fd_ >= 0 && state_ == LinkState::Connected) {
		std::string err;
		writeAll("$DIAGOFF\n", err);  // both subscriptions, in the order they were made
		writeAll("$LOGOFF\n", err);   // best effort: we are closing either way
		poll();                       // take the confirmations if they are already there
		note("disconnected from " + endpoint_.text());
	}
	closeSocket();
	state_ = LinkState::Offline;
}

bool TcpClient::writeAll(const std::string &data, std::string &err)
{
	std::size_t sent = 0;
	while (sent < data.size()) {
		const ssize_t n = ::send(fd_, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			err = errnoText(errno);
			return false;
		}
		if (n == 0) {
			err = "the engine closed the connection";
			return false;
		}
		sent += (std::size_t)n;
	}
	return true;
}

bool TcpClient::send(const std::string &line, std::string &err)
{
	if (!connected()) {
		err = "not connected: nothing was sent";
		last_error_ = err;
		return false;
	}
	if (line.find('\n') != std::string::npos || line.find('\r') != std::string::npos) {
		err = "a command is ONE line: this text carries a line break, and sending it "
		      "would run two commands where you asked for one";
		last_error_ = err;
		return false;
	}
	std::string werr;
	if (!writeAll(line + "\n", werr)) {
		err = "could not send to " + endpoint_.text() + ": " + werr;
		last_error_ = err;
		state_ = LinkState::Failed;
		closeSocket();
		return false;
	}
	++lines_sent_;
	note("> " + line);
	return true;
}

void TcpClient::records(const std::string &chunk)
{
	// The record separator is the NUL the server's send() writes; within a
	// record, the engine's own '\n' separates lines of one answer.
	// A line the engine labelled `$DIAG|` is a diagnostic; every other line is a
	// record like any other. The test is on the engine's own label, not on the
	// content (sc_tcpclient.hpp, FeedKind).
	std::string line;
	for (const char c : chunk) {
		if (c == '\n' || c == '\r') {
			if (!line.empty())
				push(kindOf(line), line);
			line.clear();
			continue;
		}
		line.push_back(c);
	}
	if (!line.empty())
		push(kindOf(line), line);
}

FeedKind TcpClient::kindOf(const std::string &line)
{
	return parseFeedDiagnostic(line).ok ? FeedKind::Diagnostic : FeedKind::Engine;
}

std::size_t TcpClient::poll()
{
	if (fd_ < 0)
		return 0;
	const std::size_t before = feed_.size() + dropped_;
	for (;;) {
		pollfd p{};
		p.fd = fd_;
		p.events = POLLIN;
		const int r = ::poll(&p, 1, 0);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			last_error_ = "poll: " + errnoText(errno);
			state_ = LinkState::Failed;
			closeSocket();
			break;
		}
		if (r == 0)
			break;
		char buf[8192];
		const ssize_t n = ::recv(fd_, buf, sizeof(buf), 0);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			last_error_ = "receive: " + errnoText(errno);
			state_ = LinkState::Failed;
			closeSocket();
			break;
		}
		if (n == 0) {
			// The engine closed the connection -- it exited, or something else
			// took it down. This is the one connection event that arrives
			// without anybody asking, and it must be visible.
			last_error_ = "the engine closed the connection";
			note("the engine closed the connection");
			state_ = LinkState::Failed;
			closeSocket();
			break;
		}
		bytes_in_ += (std::size_t)n;
		partial_.append(buf, (std::size_t)n);
		std::size_t start = 0;
		for (;;) {
			const std::size_t nul = partial_.find('\0', start);
			if (nul == std::string::npos)
				break;
			records(partial_.substr(start, nul - start));
			start = nul + 1;
		}
		partial_.erase(0, start);
	}
	return feed_.size() + dropped_ - before;
}

std::size_t TcpClient::pollFor(int ms, std::size_t want)
{
	const long deadline = nowMs() + (ms > 0 ? ms : 0);
	std::size_t got = poll();
	while ((want == 0 || got < want) && nowMs() < deadline) {
		if (fd_ < 0)
			break;
		pollfd p{};
		p.fd = fd_;
		p.events = POLLIN;
		const long left = deadline - nowMs();
		const int r = ::poll(&p, 1, (int)(left > 50 ? 50 : (left > 0 ? left : 0)));
		if (r < 0 && errno == EINTR)
			continue;
		got += poll();
	}
	return got;
}

void TcpClient::note(const std::string &text)
{
	push(FeedKind::Local, text);
}

void TcpClient::push(FeedKind kind, const std::string &text)
{
	// Counted here rather than in the pane: the bound below discards old lines,
	// and a refusal that scrolled off still happened.
	if (kind == FeedKind::Diagnostic)
		++diagnostics_in_;
	feed_.push_back(FeedLine{kind, text});
	trim();
}

void TcpClient::trim()
{
	if (bound_ == 0)
		return;
	while (feed_.size() > bound_) {
		feed_.pop_front();
		++dropped_;
	}
}

void TcpClient::clearFeed()
{
	feed_.clear();
	// `dropped_` is NOT reset: it counts what was lost over the client's life,
	// and clearing the view does not un-lose it.
}

} // namespace scedit
