/*
 * scedit — sc_tcpclient.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * One socket to a RUNNING spacecrafter, so the script under the caret can be
 * tried on the dome it was written for instead of guessed at. The engine's
 * control channel is a line protocol on TCP (`io:enable_tcp`, shipped true,
 * `io:tcp_port_in` = 7805, `claude/capability-surface.md` §1 row 2): a client
 * writes a command line, and the engine executes it exactly as it executes a
 * line of a script file.
 *
 * WHAT COMES BACK — AND WHAT DOES NOT (measured at the source, not assumed)
 * ========================================================================
 * Two things, and only two, ever reach a client:
 *   - the answer to `get status …` and to `search name …`
 *     `[observed: src/interfaceModule/app_command_interface.cpp:1284-1301,1415
 *      — the only callers of ServerSocket::setOutput in the whole tree]`;
 *   - the control replies to `$NOTICE` / `$LOGON` / `$LOGOFF`
 *     `[observed: src/tools/io.cpp:640-663]`.
 * EVERYTHING ELSE IS SILENT. A `flag stars on` that worked and a `flag stars
 * onn` that did not are the same nothing on this wire: the refusal is written
 * to the script log at L_DEBUG (INTENT §5.117) and the `$LOGON` subscription,
 * whose greeting promises the logs, actually carries other clients' command
 * answers and no log line at all (INTENT §5.72). There is also NO
 * notification when a played script ends. So a client cannot treat silence as
 * failure, cannot treat it as success, and cannot wait on an end-of-script
 * event; the editor's `#!` reload therefore watches the FILE, on a bounded
 * poll, and says so (sc_editcore.hpp § THE ENGINE WRITES BACK).
 *
 * An answer is delivered to the connection that asked AND to every `$LOGON`
 * subscriber, the addressee excluded so a client that is both gets one copy
 * `[observed: io.cpp:694-726, INTENT §11.135]`. This client subscribes on
 * connect, so its feed carries its own answers and everyone else's — which is
 * what makes it a FEED and not merely a reply channel.
 *
 * FRAMING, EXACTLY AS THE SERVER WRITES IT
 * ========================================
 * `ServerSocket::send` writes `strlen(data) + 1` bytes — the payload AND its
 * terminating NUL `[observed: io.cpp:743-756]` — and `deliver` builds the payload
 * as `answer + '\n'`. So a record on the wire is NUL-terminated and normally
 * ends with a newline; `$NOTICE`'s reply carries no newline at all. This
 * client therefore frames on the NUL, then splits a record's own newlines into
 * feed lines. Bytes that arrive after the last NUL are an incomplete record and
 * are kept until the rest of it comes.
 *
 * BYTES, NOT TEXT
 * ===============
 * ISO-8859 in both directions, never decoded (sc_document.hpp's rule, applied
 * to the wire): what the editor holds is what is sent, and what arrives is
 * shown by the same byte->cell map the buffer uses. A latin-1 accented byte in
 * a `text` command must reach the engine as that byte.
 *
 * NOTHING IS SENT WITHOUT AN EXPLICIT ACTION
 * ==========================================
 * This class has no timer, no retry and no reconnect. `connect`, `send`,
 * `disconnect` happen because a person or a tool call asked for them; `poll`
 * only reads. The one thing the editor does on its own is READ the file the
 * engine may have rewritten (sc_editcore.hpp), and that touches no socket.
 *
 * OWNERSHIP (I5)
 * ==============
 * A TcpClient owns its file descriptor and closes it in the destructor. It
 * holds NO reference to an EditCore, a Document or a Grammar, and EditCore
 * holds none to it: the editor layer owns both and moves bytes between them.
 * That is deliberate — the buffer's correctness must not depend on a socket's
 * lifetime, and a headless test of either needs nothing of the other. The
 * references `feed()` returns die with the next `poll()` or with the client.
 */

#ifndef SCEDIT_SC_TCPCLIENT_HPP
#define SCEDIT_SC_TCPCLIENT_HPP

#include <cstddef>
#include <deque>
#include <string>

namespace scedit {

//! Where a live engine listens. The defaults are the shipped ones
//! (`io:tcp_port_in` = 7805 on the loopback), so `--tcp` alone is enough for
//! the ordinary case of an engine on the same machine.
struct Endpoint {
	std::string host = "127.0.0.1";
	int port = 7805;
	std::string text() const { return host + ":" + std::to_string(port); }
};

//! Parse `[host:]port` — `7805`, `127.0.0.1:7805`, `dome:7805`. A bare number
//! is a port on the default host. Returns false and fills `err` with a sentence
//! naming what was wrong and what the two accepted shapes are.
bool parseEndpoint(const std::string &spec, Endpoint &out, std::string &err);

//! What the connection is, as DATA — the editor shows it and the tests assert
//! it. `Failed` remembers that the last attempt failed, which is not the same
//! thing as never having tried (the status line says so, and `lastError()`
//! carries the reason).
enum class LinkState {
	Offline,     //!< no socket, nothing attempted since the last disconnect
	Connected,   //!< a socket is open and $LOGON was sent
	Failed       //!< the last connect or send failed; see lastError()
};

//! Where a feed line came from, as far as this layer can honestly tell.
//! The wire does not label its records, so this is not a parse of content: it
//! is the CONTROL replies (which are answers to what this client just asked)
//! told apart from everything else. Anything richer would be scedit inventing
//! a protocol the engine does not speak.
enum class FeedKind {
	Engine,      //!< a record the engine sent
	Local        //!< a line scedit wrote into the feed itself (what it sent, and why)
};

struct FeedLine {
	FeedKind kind = FeedKind::Engine;
	std::string text;   //!< bytes, terminators stripped, never decoded
};

class TcpClient {
public:
	TcpClient() = default;
	~TcpClient();
	TcpClient(const TcpClient &) = delete;
	TcpClient &operator=(const TcpClient &) = delete;

	//! Open the connection and subscribe to the feedback channel with `$LOGON`.
	//! Returns false and fills `err` (also stored in `lastError`) on failure;
	//! the state is then `Failed`. Connecting while connected is a no-op that
	//! returns true.
	bool connect(const Endpoint &ep, std::string &err);
	//! `$LOGOFF`, then close. Safe to call when not connected.
	void disconnect();

	LinkState state() const { return state_; }
	bool connected() const { return state_ == LinkState::Connected; }
	const Endpoint &endpoint() const { return endpoint_; }
	const std::string &lastError() const { return last_error_; }

	//! Send ONE command line. The bytes are sent unchanged with a '\n'
	//! appended, which is the engine's line terminator; an embedded newline is
	//! refused rather than silently split, because two commands are not one
	//! command and the caller must know which it sent.
	bool send(const std::string &line, std::string &err);

	//! Read whatever has arrived, without blocking, and append it to the feed.
	//! Returns the number of feed lines added. A closed connection (the engine
	//! exited) is detected here: the state becomes Failed and the reason is in
	//! lastError().
	std::size_t poll();
	//! Poll until at least `want` lines have been added or `ms` milliseconds
	//! have passed, whichever comes first. Returns the number added. `want` = 0
	//! means "read for the whole time" — which is what a caller uses when it
	//! cannot know how many records an answer is.
	std::size_t pollFor(int ms, std::size_t want = 0);

	//! Write a line into the feed WITHOUT sending anything: what scedit did,
	//! shown in the same place as what the engine said, so the pane reads as a
	//! conversation. Marked Local — nothing here ever came off the wire.
	void note(const std::string &text);

	//! The feed, oldest first, bounded (see setFeedBound).
	const std::deque<FeedLine> &feed() const { return feed_; }
	//! How many lines the bound has discarded since the client was made. A
	//! bounded buffer that drops silently is a buffer that lies about what
	//! happened, so the count is kept and the pane shows it.
	std::size_t dropped() const { return dropped_; }
	//! The bound, in LINES. Default 500: a long enough scrollback to read an
	//! answer that arrived while you were typing, small enough that an engine
	//! broadcasting to a subscriber cannot grow the editor's memory without
	//! limit (this is a feed of other clients' traffic too — its rate is not
	//! scedit's to control). A bound of 0 means unbounded and is never the
	//! default.
	void setFeedBound(std::size_t lines) { bound_ = lines; trim(); }
	std::size_t feedBound() const { return bound_; }
	void clearFeed();

	//! Bytes received since the connection opened, and lines sent: two counters
	//! the status line shows, so "connected but nothing is happening" and
	//! "connected and the engine is talking" are distinguishable.
	std::size_t bytesIn() const { return bytes_in_; }
	std::size_t linesSent() const { return lines_sent_; }

private:
	int fd_ = -1;
	LinkState state_ = LinkState::Offline;
	Endpoint endpoint_;
	std::string last_error_;
	std::string partial_;        //!< bytes after the last NUL: an incomplete record
	std::deque<FeedLine> feed_;
	std::size_t bound_ = 500;
	std::size_t dropped_ = 0;
	std::size_t bytes_in_ = 0;
	std::size_t lines_sent_ = 0;

	void push(FeedKind kind, const std::string &text);
	void trim();
	//! Split a complete record into feed lines. Public behaviour is tested
	//! through poll(); this exists so the framing rule lives in one place.
	void records(const std::string &chunk);
	bool writeAll(const std::string &data, std::string &err);
	void closeSocket();
};

} // namespace scedit

#endif // SCEDIT_SC_TCPCLIENT_HPP
