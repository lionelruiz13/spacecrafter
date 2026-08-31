/*
 * tests/tcpclient_test.cpp — sc_tcpclient against a stand-in engine.
 *
 * Driven by tests/tcp_gate.py, one LEG per process: the gate starts the fake
 * engine (tests/fake_engine.py, whose framing rules are read from the engine's
 * own source), runs this binary with the leg's name and the endpoint, and then
 * asserts SERVER-SIDE what arrived — so each leg is checked from both ends and
 * a client that quietly sends nothing cannot pass by agreeing with itself.
 *
 *     tcpclient_test <leg> [host:port]
 *
 * Exit 0 when every check of the leg passed, 1 otherwise, 2 on misuse.
 */

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "sc_tcpclient.hpp"

using namespace scedit;

static int failures = 0;
static int checks = 0;

static void check(bool ok, const std::string &what)
{
	++checks;
	if (!ok)
		++failures;
	std::printf("  %s  %s\n", ok ? "ok    " : "FAIL  ", what.c_str());
}

//! Every feed line, one string per entry, with the local ones marked — what a
//! leg asserts on.
static std::vector<std::string> engineLines(const TcpClient &c)
{
	std::vector<std::string> out;
	for (const FeedLine &f : c.feed())
		if (f.kind == FeedKind::Engine)
			out.push_back(f.text);
	return out;
}

static bool contains(const std::vector<std::string> &v, const std::string &s)
{
	for (const auto &x : v)
		if (x == s)
			return true;
	return false;
}

//! Every feed line, engine and local alike.
static std::vector<std::string> allLines(const TcpClient &c)
{
	std::vector<std::string> out;
	for (const FeedLine &f : c.feed())
		out.push_back(f.text);
	return out;
}

static bool containsSub(const std::vector<std::string> &v, const std::string &s)
{
	for (const auto &x : v)
		if (x.find(s) != std::string::npos)
			return true;
	return false;
}

// --------------------------------------------------------------- the legs

//! Pure parsing: no socket at all. `[host:]port`, and the refusals.
static void legParse()
{
	Endpoint ep;
	std::string err;
	check(parseEndpoint("7805", ep, err) && ep.host == "127.0.0.1" && ep.port == 7805,
	      "a bare port is a port on the default host 127.0.0.1");
	check(parseEndpoint("192.168.0.9:7805", ep, err) && ep.host == "192.168.0.9" && ep.port == 7805,
	      "host:port is split at the last colon");
	check(parseEndpoint("dome:1", ep, err) && ep.host == "dome" && ep.port == 1,
	      "a host NAME is accepted (resolution is the connect's business)");
	check(ep.text() == "dome:1", "an endpoint prints as host:port");
	check(!parseEndpoint("", ep, err) && err.find("host:port") != std::string::npos,
	      "an empty spec is refused, and the message says what the shapes are");
	check(!parseEndpoint("seven", ep, err) && err.find("not a port number") != std::string::npos,
	      "a non-numeric port is refused by name");
	check(!parseEndpoint("host:", ep, err), "a missing port is refused");
	check(!parseEndpoint(":7805", ep, err) && err.find("empty") != std::string::npos,
	      "an empty host is refused rather than silently defaulted");
	check(!parseEndpoint("7805x", ep, err), "trailing rubbish after the port is refused");
	check(!parseEndpoint("0", ep, err) && !parseEndpoint("65536", ep, err),
	      "a port outside 1-65535 is refused");
	// The default is the shipped one, and that is a fact about the ENGINE:
	// io:tcp_port_in = 7805 (capability-surface §1 row 2).
	Endpoint d;
	check(d.host == "127.0.0.1" && d.port == 7805,
	      "the default endpoint is the shipped one: 127.0.0.1:7805");
}

//! Nothing is listening: the failure is reported, not thrown, and it says what
//! to check.
static void legRefused(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.state() == LinkState::Offline, "a fresh client is Offline");
	check(!c.connect(ep, err), "connecting where nothing listens fails");
	check(c.state() == LinkState::Failed, "and the state says Failed, not Offline");
	check(err.find(ep.text()) != std::string::npos, "the message names the endpoint");
	check(err.find("io:enable_tcp") != std::string::npos,
	      "and names the engine setting to check (§2(f): what, consequence, prevention)");
	check(c.lastError() == err, "lastError carries the same sentence");
	std::string serr;
	check(!c.send("flag stars on", serr) && serr.find("not connected") != std::string::npos,
	      "sending while offline is refused and sends nothing");
	check(c.poll() == 0, "polling while offline yields nothing");
	c.disconnect();   // must not crash, must not throw
	check(c.state() == LinkState::Offline, "disconnect on an offline client is a no-op");
}

//! Connect, subscribe, ask, be answered, disconnect, reconnect.
static void legBasic(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	check(c.connected() && c.state() == LinkState::Connected, "the state is Connected");
	c.pollFor(2000, 1);
	check(containsSub(engineLines(c), "Vous receverez maintenant les logs"),
	      "the $LOGON subscription is confirmed by the engine");
	check(containsSub(engineLines(c), "logs"), "the confirmation reached the FEED, not a log");

	const std::size_t before = c.feed().size();
	check(c.send("get status position", err), "send a get: " + err);
	c.pollFor(2000, 1);
	const std::vector<std::string> lines = engineLines(c);
	check(containsSub(lines, "2461233.5"),
	      "the answer to `get status position` arrives on THIS connection (§5.47/§11.135)");
	check(c.feed().size() > before + 1,
	      "the feed carries both what was sent (a local line) and what came back");

	// $NOTICE's reply carries NO trailing newline: a client that framed on the
	// newline alone would hold it forever.
	check(c.send("$NOTICE", err), "send $NOTICE");
	c.pollFor(2000, 1);
	check(contains(engineLines(c), "$NOTICE $LOGON $LOGOFF"),
	      "a record with no trailing newline is still a complete feed line");

	// A command that is not `get`/`search` produces NOTHING on the wire. That
	// is the engine's shape, and an editor must not present silence as either
	// success or failure (sc_tcpclient.hpp § WHAT COMES BACK).
	const std::size_t quiet = engineLines(c).size();
	check(c.send("flag stars on", err), "send an ordinary command");
	c.pollFor(400);
	check(engineLines(c).size() == quiet,
	      "an ordinary command is answered with silence: the feed gains nothing");

	// Three: the subscription is not a command and is not counted, and the
	// counter is per CONNECTION — it starts again at the reconnect below.
	// (Written `== 4` first, on my own miscount; the gate said 3.)
	check(c.linesSent() == 3, "three command lines were sent; $LOGON is not one of them");
	check(c.bytesIn() > 0, "bytes were received");

	c.disconnect();
	check(c.state() == LinkState::Offline, "after disconnect the state is Offline");
	std::string serr;
	check(!c.send("flag stars off", serr), "and nothing can be sent through it");

	// Reconnect on the same object: a new connection, a new subscription.
	check(c.connect(ep, err), "reconnect: " + err);
	check(c.connected(), "the reconnected client is Connected");
	c.pollFor(2000, 1);
	check(c.send("get status constellation", err), "and it can ask again");
	c.pollFor(2000, 1);
	check(contains(engineLines(c), "UMa"), "the answer arrives on the NEW connection");
	c.disconnect();
}

//! A line break in a command is refused: two commands are not one command.
static void legNewline(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 1);
	const std::size_t sent = c.linesSent();
	std::string serr;
	check(!c.send("flag stars on\nflag planets on", serr), "a two-line command is refused");
	check(serr.find("ONE line") != std::string::npos, "the message says why");
	check(c.linesSent() == sent, "and NOTHING was sent (the counter did not move)");
	check(c.connected(), "the connection survives a refused send");
	check(c.send("get status media", err), "the client still works afterwards");
	c.pollFor(2000, 1);
	check(contains(engineLines(c), "NMF"), "and is answered");
	c.disconnect();
}

//! ISO-8859 bytes go out unchanged. The gate checks the server side.
static void legLatin1(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 1);
	// 0xE9 is 'é' in ISO-8859-1 and is not valid UTF-8 on its own: a client
	// that decoded its input would mangle or refuse this.
	const std::string line = "text name caf\xE9 string \"caf\xE9 \xA0 x\"";
	check(c.send(line, err), "a latin-1 command line is sent: " + err);
	c.pollFor(300);
	c.disconnect();
}

//! The feed is bounded, and what the bound discards is COUNTED.
static void legBound(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.feedBound() == 500, "the default bound is 500 lines");
	c.setFeedBound(4);
	check(c.feedBound() == 4, "the bound is settable");
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 1);
	for (int i = 0; i < 10; ++i)
		c.send("get status position", err);
	c.pollFor(3000, 0);
	check(c.feed().size() == 4, "the feed never exceeds the bound");
	check(c.dropped() >= 10, "and every discarded line is counted, not lost in silence");
	const std::size_t d = c.dropped();
	c.clearFeed();
	check(c.feed().empty() && c.dropped() == d,
	      "clearing the view does not un-drop what was dropped");
	c.disconnect();
}

//! Another client asks; this one is subscribed and sees the answer. That is
//! the $LOGON channel's actual semantics (INTENT §5.72), and it is why the
//! pane is a FEED.
static void legFeed(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 1);
	// The marker tells the gate to have a SECOND client ask a question.
	check(c.send("get status object", err), "marker sent");
	c.pollFor(2000, 1);
	check(contains(engineLines(c), "EOL"), "our own answer arrived");
	// The gate now drives the other client; we wait for its answer to reach us.
	c.pollFor(6000, 1);
	check(contains(engineLines(c), "M1|M10|M100"),
	      "ANOTHER client's answer arrives on our feed because we subscribed with $LOGON");
	c.disconnect();
}

//! The engine goes away. Nothing else in this class ever changes state without
//! being asked; this is the one event that arrives on its own, and it must be
//! visible rather than turn into a silent dead socket.
static void legClosed(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 1);
	check(c.send("get status position", err), "marker sent (the gate stops the engine now)");
	c.pollFor(8000, 2);
	// Poll until the close is seen or the budget runs out.
	for (int i = 0; i < 100 && c.connected(); ++i)
		c.pollFor(100);
	check(!c.connected(), "the client notices that the engine closed the connection");
	check(c.state() == LinkState::Failed, "the state is Failed, not Offline");
	check(c.lastError().find("closed") != std::string::npos, "and says so: " + c.lastError());
	// A LOCAL line: nothing came off the wire to say this — the wire went
	// away. The pane must still show it, which is why the feed carries both
	// kinds. (Asserted against engineLines() first, and it failed, correctly.)
	check(containsSub(allLines(c), "closed") && !containsSub(engineLines(c), "closed"),
	      "the feed carries the event as a LOCAL line — the engine said nothing, scedit did");
	std::string serr;
	check(!c.send("flag stars on", serr), "nothing can be sent after that");
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::fprintf(stderr, "usage: tcpclient_test <leg> [host:port]\n");
		return 2;
	}
	const std::string leg = argv[1];
	Endpoint ep;
	if (argc > 2) {
		std::string err;
		if (!parseEndpoint(argv[2], ep, err)) {
			std::fprintf(stderr, "tcpclient_test: %s\n", err.c_str());
			return 2;
		}
	}
	std::printf("=== leg %s (%s)\n", leg.c_str(), ep.text().c_str());
	if (leg == "parse") legParse();
	else if (leg == "refused") legRefused(ep);
	else if (leg == "basic") legBasic(ep);
	else if (leg == "newline") legNewline(ep);
	else if (leg == "latin1") legLatin1(ep);
	else if (leg == "bound") legBound(ep);
	else if (leg == "feed") legFeed(ep);
	else if (leg == "closed") legClosed(ep);
	else {
		std::fprintf(stderr, "tcpclient_test: no such leg: %s\n", leg.c_str());
		return 2;
	}
	std::printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
