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
 * THREE things reach a client, and the third one is new:
 *   - the answer to `get status …` and to `search name …`
 *     `[observed: src/interfaceModule/app_command_interface.cpp:1309-1326,1440
 *      — the only callers of ServerSocket::setOutput in the whole tree]`;
 *   - the control replies to `$NOTICE` / `$LOGON` / `$LOGOFF`, and now to
 *     `$DIAGON` / `$DIAGOFF` `[observed: src/tools/io.cpp, computeNormalString]`;
 *   - since engine `be2ddd81` (INTENT 11.188): on a connection that sent
 *     `$DIAGON`, one record per DIAGNOSTIC the engine produces about a command
 *     it read on the control socket - the refusals it used to write only into
 *     its own log. This client subscribes on connect, so a `flag stars onn`
 *     that was refused now says so.
 *
 * ~~EVERYTHING ELSE IS SILENT.~~ That was the whole story until 11.188, and
 * the part of it that still holds is worth keeping straight, because an older
 * engine is a real target:
 *   - SUCCESS is still silent. `flag stars on` that WORKED sends nothing, so
 *     silence still cannot be read as success - only as "no diagnostic".
 *   - a script's LIFECYCLE is still silent: no event when a play starts, none
 *     when it ends (11.185). The editor's `#!` reload therefore still watches
 *     the FILE, on a bounded poll, and says so (sc_editcore.hpp section THE ENGINE
 *     WRITES BACK).
 *   - a refusal produced INSIDE another command (the engine's nested calls,
 *     `media action play ...` -> `audio filename ...`) carries no origin, so it
 *     routes nowhere and does not arrive `[measured: claude/harness/
 *     f69_feedback.py leg vi]`.
 *   - against an engine older than `be2ddd81`, `$DIAGON` is not a verb: it is
 *     an unrecognised command, refused into that engine's log, and nothing
 *     comes back. Connecting is unharmed; the feed is simply as quiet as it
 *     always was.
 * The refusals are also still written to the script log at L_DEBUG (INTENT
 * 5.117) - the wire carries a COPY. And the `$LOGON` subscription, whose
 * greeting promises the logs, still carries other clients' command answers and
 * no log line at all (INTENT 5.72): the two subscriptions are different
 * things and this client holds both.
 *
 * THE DIAGNOSTIC RECORD, EXACTLY
 * ==============================
 * `$DIAG|<origin>|<message>|<subject>` - four fields, engine-controlled first,
 * so the SUBJECT (a command line, which may itself contain a `|`) is the last
 * and the split is bounded at three. `<origin>` is `tcp#<id>`, the engine's
 * never-reused connection id: a diagnostic caused by ANOTHER client on the same
 * engine arrives here too, tagged with its id and not with ours. Reading it as
 * "my command failed" without checking the origin is the mistake this field
 * exists to prevent.
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
//! `FeedKind::Diagnostic` is the one content test in here, and it is a test the ENGINE
//! made available: a record that begins `$DIAG|` is labelled as a diagnostic by
//! the engine itself (INTENT 11.188), so telling it apart is reading the
//! protocol rather than guessing at it. Everything else stays what it was - the
//! records the engine sent, and the lines scedit wrote itself. Anything richer
//! would be scedit inventing a protocol the engine does not speak.
enum class FeedKind {
	Engine,      //!< a record the engine sent
	Diagnostic,  //!< a record the engine sent AND labelled `$DIAG|...`: something
	             //!< it refused, and why. Still an engine record - a reader that
	             //!< wants "everything the engine said" takes both.
	Local        //!< a line scedit wrote into the feed itself (what it sent, and why)
};

//! A `$DIAG|<origin>|<message>|<subject>` record, split. `ok` is false when the
//! line did not have that shape, and then the other fields are empty and the
//! caller should use the raw text: a malformed record is shown, never dropped.
struct FeedDiagnostic {
	bool ok = false;
	std::string origin;    //!< `tcp#<id>` - WHICH connection caused it, not necessarily ours
	std::string message;   //!< the engine's own words
	std::string subject;   //!< the command line it is about (may contain `|`)
};

//! Split one feed line into its diagnostic fields. Returns `ok == false` for
//! any line that is not a `$DIAG|` record.
FeedDiagnostic parseFeedDiagnostic(const std::string &line);

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

	//! Open the connection and subscribe to BOTH channels: `$LOGON` for the
	//! answer feed and `$DIAGON` for the diagnostics about commands sent on the
	//! control socket. Two verbs because they are two subscriptions in the
	//! engine - `$LOGON`'s is the one a closed-source client may also be on and
	//! which therefore never changed; `$DIAGON`'s is the link this tool was
	//! given (INTENT 11.186(c), 11.188).
	//! Only `$LOGON` failing is fatal: `$DIAGON` on an engine that predates it
	//! is an unrecognised command, which costs nothing and must not stop a
	//! connection that would otherwise work.
	//! Returns false and fills `err` (also stored in `lastError`) on failure;
	//! the state is then `Failed`. Connecting while connected is a no-op that
	//! returns true.
	bool connect(const Endpoint &ep, std::string &err);
	//! `$DIAGOFF`, `$LOGOFF`, then close. Safe to call when not connected.
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
	//! How many diagnostics have arrived since the connection opened. Counted
	//! over the connection's life, not over the feed, so the bound discarding
	//! old lines does not un-count a refusal that happened.
	std::size_t diagnosticsIn() const { return diagnostics_in_; }

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
	std::size_t diagnostics_in_ = 0;

	void push(FeedKind kind, const std::string &text);
	//! Which kind a received line is: the engine's `$DIAG|` label, or nothing.
	static FeedKind kindOf(const std::string &line);
	void trim();
	//! Split a complete record into feed lines. Public behaviour is tested
	//! through poll(); this exists so the framing rule lives in one place.
	void records(const std::string &chunk);
	bool writeAll(const std::string &data, std::string &err);
	void closeSocket();
};

} // namespace scedit

#endif // SCEDIT_SC_TCPCLIENT_HPP
