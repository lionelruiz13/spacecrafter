/*
 * tests/tcpclient_test.cpp -- sc_tcpclient against a stand-in engine.
 *
 * Driven by tests/tcp_gate.py, one LEG per process: the gate starts the fake
 * engine (tests/fake_engine.py, whose framing rules are read from the engine's
 * own source), runs this binary with the leg's name and the endpoint, and then
 * asserts SERVER-SIDE what arrived -- so each leg is checked from both ends and
 * a client that quietly sends nothing cannot pass by agreeing with itself.
 *
 *     tcpclient_test <leg> [host:port] [more...]
 *
 * THE `live_*` LEGS ARE FOR A REAL ENGINE, and they are here rather than in a
 * separate binary because they must be the SAME client and the same core calls
 * (I2): `claude/harness/f67_tcp_live.py` launches spacecrafter on a temp-HOME
 * farm and runs them over port 7805, reading the engine's side of each claim
 * through a channel this binary does not touch (the session file, the script
 * log, `scedit --history`). What they do NOT drive is the terminal: they make
 * the calls the editor's keys make, in the editor's order, so a claim about a
 * KEY rests on the ui gate's frames plus this -- stated in the delivery rather
 * than glossed over.
 *
 * Exit 0 when every check of the leg passed, 1 otherwise, 2 on misuse.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>
#include <vector>

#include "sc_editcore.hpp"
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

//! Every feed line, one string per entry, with the local ones marked -- what a
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

//! Does any of these lines BEGIN with this? Not the same question as `contains
//! a substring`, and the difference matters for `$DIAG|`: the subscription's own
//! confirmation MENTIONS the record shape in its text.
static bool startsWithAny(const std::vector<std::string> &v, const std::string &s)
{
	for (const auto &x : v)
		if (x.compare(0, s.size(), s) == 0)
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
	// io:tcp_port_in = 7805 (capability-surface S1 row 2).
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
	      "and names the engine setting to check (\xc2\xa7" "2(f): what, consequence, prevention)");
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
	c.pollFor(2000, 2);
	check(containsSub(engineLines(c), "Vous receverez maintenant les logs"),
	      "the $LOGON subscription is confirmed by the engine");
	check(containsSub(engineLines(c), "logs"), "the confirmation reached the FEED, not a log");

	const std::size_t before = c.feed().size();
	check(c.send("get status position", err), "send a get: " + err);
	c.pollFor(2000, 1);
	const std::vector<std::string> lines = engineLines(c);
	check(containsSub(lines, "2461233.5"),
	      "the answer to `get status position` arrives on THIS connection (\xc2\xa7" "5.47/\xc2\xa7" "11.135)");
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
	// success or failure (sc_tcpclient.hpp S WHAT COMES BACK).
	const std::size_t quiet = engineLines(c).size();
	check(c.send("flag stars on", err), "send an ordinary command");
	c.pollFor(400);
	check(engineLines(c).size() == quiet,
	      "an ordinary command is answered with silence: the feed gains nothing");

	// Three: the subscription is not a command and is not counted, and the
	// counter is per CONNECTION -- it starts again at the reconnect below.
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
	c.pollFor(2000, 2);
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
	c.pollFor(2000, 2);
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
	c.pollFor(2000, 2);
	// 0xE9 is 'e' in ISO-8859-1 and is not valid UTF-8 on its own: a client
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
	c.pollFor(2000, 2);
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
//! the $LOGON channel's actual semantics (INTENT S5.72), and it is why the
//! pane is a FEED.
static void legFeed(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 2);
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
	c.pollFor(2000, 2);
	check(c.send("get status position", err), "marker sent (the gate stops the engine now)");
	c.pollFor(8000, 2);
	// Poll until the close is seen or the budget runs out.
	for (int i = 0; i < 100 && c.connected(); ++i)
		c.pollFor(100);
	check(!c.connected(), "the client notices that the engine closed the connection");
	check(c.state() == LinkState::Failed, "the state is Failed, not Offline");
	check(c.lastError().find("closed") != std::string::npos, "and says so: " + c.lastError());
	// A LOCAL line: nothing came off the wire to say this -- the wire went
	// away. The pane must still show it, which is why the feed carries both
	// kinds. (Asserted against engineLines() first, and it failed, correctly.)
	check(containsSub(allLines(c), "closed") && !containsSub(engineLines(c), "closed"),
	      "the feed carries the event as a LOCAL line \xe2\x80\x94 the engine said nothing, scedit did");
	std::string serr;
	check(!c.send("flag stars on", serr), "nothing can be sent after that");
}

//! Every DIAGNOSTIC feed line, split into its fields.
static std::vector<FeedDiagnostic> diagnostics(const TcpClient &c)
{
	std::vector<FeedDiagnostic> out;
	for (const FeedLine &f : c.feed())
		if (f.kind == FeedKind::Diagnostic)
			out.push_back(parseFeedDiagnostic(f.text));
	return out;
}

//! The dedicated diagnostic link, against the stand-in: subscribed on connect,
//! a `$DIAG|` record told apart from an answer, and a malformed one SHOWN.
static void legDiag(const Endpoint &ep)
{
	// Parsing first, with no socket in it: the record's shape is a contract and
	// a broken record must not become an invisible one.
	const FeedDiagnostic d = parseFeedDiagnostic("$DIAG|tcp#7|command 'get': unknown status value|"
	                                     "get status nonsense");
	check(d.ok && d.origin == "tcp#7" && d.message == "command 'get': unknown status value"
	      && d.subject == "get status nonsense", "a well-formed record splits into four fields");
	const FeedDiagnostic pipe = parseFeedDiagnostic("$DIAG|tcp#7|bad|text string \"a|b\"");
	check(pipe.ok && pipe.subject == "text string \"a|b\"",
	      "a SUBJECT containing the separator survives: the split is bounded at three");
	check(!parseFeedDiagnostic("$DIAG|tcp#7").ok && !parseFeedDiagnostic("Vous receverez").ok
	      && !parseFeedDiagnostic("$DIAG").ok && !parseFeedDiagnostic("").ok,
	      "a truncated or foreign line is NOT a diagnostic");

	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 2);
	check(containsSub(allLines(c), "$LOGON and $DIAGON"),
	      "the local note says BOTH subscriptions were sent");
	check(containsSub(engineLines(c), "$DIAGON ok:"),
	      "and the engine confirmed the diagnostic one");
	check(c.diagnosticsIn() == 0,
	      "the confirmation is NOT counted as a diagnostic: it is an answer to a verb");

	// One diagnostic, pushed by the stand-in exactly as the engine pushes one.
	check(c.send("flagg stars on", err), "send a command the engine will refuse: " + err);
	c.pollFor(2000, 1);
	const std::vector<FeedDiagnostic> got = diagnostics(c);
	check(got.size() == 1, "exactly one diagnostic arrived");
	check(got.size() == 1 && got[0].origin == "tcp#1" && got[0].subject == "flagg stars on"
	      && got[0].message == "Unrecognized or malformed command name",
	      "with the origin, the message and the command line it is about");
	check(c.diagnosticsIn() == 1, "and the counter moved");
	check(!startsWithAny(engineLines(c), "$DIAG|"),
	      "a diagnostic is NOT also an ordinary engine line: the kinds are exclusive "
	      "(the $DIAGON confirmation MENTIONS the shape, which is why this tests the "
	      "start of a line and not a substring - it went red the other way first)");

	// The two channels coexist: an ANSWER still arrives, and is not a diagnostic.
	const std::size_t diagsBefore = c.diagnosticsIn();
	check(c.send("get status position", err), "ask a question too: " + err);
	c.pollFor(2000, 1);
	check(containsSub(engineLines(c), "2461233.5"), "the answer arrives as an ENGINE line");
	check(c.diagnosticsIn() == diagsBefore, "and is not mistaken for a diagnostic");

	// A malformed record is SHOWN rather than dropped: a client that hides what
	// it cannot parse is a client that hides a change of protocol. The gate
	// pushes `$DIAG|tcp#1|truncated` - a prefix without its third separator.
	c.pollFor(3000, 1);
	check(containsSub(allLines(c), "$DIAG|tcp#1|truncated"),
	      "a record that does not split is still on the feed, verbatim");
	check(c.diagnosticsIn() == diagsBefore,
	      "and it did NOT count as a diagnostic: the parse decides, not the prefix");
	c.disconnect();
}

// ------------------------------------------------- the legs for a real engine

//! Wait, at the editor's own cadence, for the file to change under the buffer.
//! This is the bounded poll sc_tui.hpp describes -- once a second, for a stated
//! window -- run here so the live instrument measures the real thing.
static bool waitForWriteBack(EditCore &core, int seconds)
{
	for (int i = 0; i < seconds; ++i) {
		if (core.diskState() == DiskState::Changed)
			return true;
		::sleep(1);
	}
	return core.diskState() == DiskState::Changed;
}

static void printHistory(const EditCore &core, const char *tag)
{
	for (const ErrorEntry &e : core.errorHistory())
		std::printf("  %s %s line %zu [%s] %s\n", tag,
		            e.source == EntrySource::Engine ? "spacecrafter" : "scedit",
		            e.line, e.id.c_str(), e.message.c_str());
}

//! THE DEDICATED LINK, AGAINST THE REAL ENGINE. Two ways, one leg: with
//! `expectDiagnostics` the engine is one that has INTENT 11.188 and a refused
//! command must come back; without it the engine PREDATES the change and the
//! same command must come back as nothing at all. The second form is not a
//! formality - it is what says the first one measured the engine rather than
//! scedit's own hopes, and it is the only leg here whose green means the
//! opposite thing.
static void legLiveDiag(const Endpoint &ep, bool expectDiagnostics)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect to the live engine: " + err);
	c.pollFor(3000, 2);
	check(containsSub(engineLines(c), "Vous receverez maintenant les logs"),
	      "the $LOGON subscription is confirmed (unchanged, both engines)");
	check(containsSub(engineLines(c), "$DIAGON ok:") == expectDiagnostics,
	      expectDiagnostics ? "and $DIAGON is confirmed too"
	                        : "and $DIAGON is NOT confirmed: this engine has no such verb");

	// A command this engine cannot recognise. On the old one it vanishes into a
	// log file nobody on this socket can read; on the new one it comes back.
	const std::size_t before = c.diagnosticsIn();
	check(c.send("flagg stars on", err), "send a command the engine will refuse: " + err);
	c.pollFor(3000, 1);
	const std::vector<FeedDiagnostic> got = diagnostics(c);
	if (expectDiagnostics) {
		check(got.size() == 1, "exactly one diagnostic came back");
		check(got.size() == 1 && got[0].subject == "flagg stars on",
		      "naming the command line it is about");
		check(got.size() == 1 && got[0].origin.compare(0, 4, "tcp#") == 0
		      && got[0].origin.size() > 4,
		      "and the connection it came from: " + (got.empty() ? "" : got[0].origin));
		check(got.size() == 1 && got[0].message.find("nrecognized") != std::string::npos,
		      "with the engine's own words, not scedit's");
		check(c.diagnosticsIn() == before + 1, "the counter moved by exactly one");
	} else {
		check(got.empty() && c.diagnosticsIn() == before,
		      "nothing came back: this engine refuses in silence, as every engine did");
	}

	// Whatever the engine's age, an ANSWER still arrives on the same socket -
	// the frozen half, checked from the client's side.
	check(c.send("get status position", err), "ask a question: " + err);
	c.pollFor(3000, 1);
	check(containsSub(engineLines(c), ";"),
	      "the `get status position` answer arrives, both engines alike");
	c.disconnect();
}

//! Send one command and leave. The engine says nothing about it, so what it did
//! is read by the harness through another channel entirely.
static void legLiveSend(const Endpoint &ep, const std::vector<std::string> &commands)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect to the live engine: " + err);
	c.pollFor(2000, 2);
	// The subscription's confirmation IS an engine line, and counting it among
	// the answers to the commands is how a count of 0 becomes a count of 1 and
	// says nothing. It is asserted here and then cleared, so what follows is
	// about the commands and nothing else.
	check(!engineLines(c).empty(), "the $LOGON subscription was confirmed by the engine");
	c.clearFeed();
	for (const std::string &command : commands) {
		check(c.send(command, err), "send `" + command + "`: " + err);
		c.pollFor(800);
		std::printf("  sent: %s\n", command.c_str());
	}
	for (const FeedLine &f : c.feed())
		std::printf("  feed[%s] %s\n", f.kind == FeedKind::Local ? "L" : "E", f.text.c_str());
	std::printf("  engine lines: %zu\n", engineLines(c).size());
	c.disconnect();
}

//! S5.47: the reply to a `get` reaches the connection that ASKED. Before F27 it
//! reached only the $LOGON subscribers; this client is both, and must get one
//! copy, not two.
static void legLiveGet(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 2);
	check(c.send("get status position", err), "send a get: " + err);
	c.pollFor(4000, 1);
	std::vector<std::string> lines = engineLines(c);
	std::size_t positions = 0;
	for (const auto &l : lines) {
		std::printf("  reply: %s\n", l.c_str());
		// The engine's shape: five ';'-separated fields (coreLink tcpGetPosition).
		std::size_t semis = 0;
		for (char ch : l)
			if (ch == ';')
				++semis;
		if (semis == 5)
			++positions;
	}
	check(positions == 1,
	      "exactly ONE position reply on this connection (issuer + subscriber = one copy)");
	c.disconnect();
}

//! The $LOGON feed: what the engine broadcasts arrives here. The harness has a
//! SECOND client ask a question while this leg waits.
static void legLiveFeed(const Endpoint &ep, int seconds)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 2);
	std::printf("  MARKER subscribed\n");
	std::fflush(stdout);
	for (int i = 0; i < seconds * 4; ++i)
		c.pollFor(250);
	for (const FeedLine &f : c.feed())
		if (f.kind == FeedKind::Engine)
			std::printf("  feed: %s\n", f.text.c_str());
	check(!engineLines(c).empty(), "something arrived on the feed");
	c.disconnect();
}

static void legLiveReconnect(const Endpoint &ep)
{
	TcpClient c;
	std::string err;
	check(c.connect(ep, err), "first connect: " + err);
	c.pollFor(2000, 2);
	check(c.send("get status position", err), "ask on the first connection");
	c.pollFor(4000, 1);
	const std::size_t first = engineLines(c).size();
	check(first > 0, "the first connection was answered");
	c.disconnect();
	check(c.state() == LinkState::Offline, "disconnected");
	check(c.connect(ep, err), "reconnect: " + err);
	c.pollFor(2000, 2);
	check(c.send("get status position", err), "ask on the second connection");
	c.pollFor(4000, 1);
	check(engineLines(c).size() > 0, "the second connection was answered too");
	std::printf("  lines after reconnect: %zu\n", engineLines(c).size());
	c.disconnect();
}

//! (d) Play the open file, let the engine write its `#!` back, and take it into
//! a CLEAN buffer -- the editor's whole write-back path, headless.
static void legLivePlay(const Endpoint &ep, const std::string &grammar,
                        const std::string &file, int seconds)
{
	EditCore core;
	std::string err;
	check(core.open(grammar, file, err), "open the script: " + err);
	check(core.diskState() == DiskState::Same, "the buffer starts in step with the file");
	check(core.engineTailCount() == 0, "and with no engine tail in it");
	printHistory(core, "before:");

	TcpClient c;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 2);
	check(!engineLines(c).empty(), "the $LOGON subscription was confirmed by the engine");
	c.clearFeed();   // from here, everything on this wire is about the script
	check(!core.dirty(), "the buffer is clean, so the play needs no save first");
	check(c.send("script action play filename " + file, err), "play the file: " + err);

	const bool changed = waitForWriteBack(core, seconds);
	check(changed, "the engine rewrote the file within the window");
	if (changed) {
		check(core.reloadFromDisk(err), "the clean buffer reloads: " + err);
		check(core.engineTailCount() > 0, "and the engine's `#!` finding(s) are listed");
		std::printf("  engine tails after reload: %zu\n", core.engineTailCount());
		printHistory(core, "after:");
	}
	// THE PROTOCOL GAP, measured rather than argued: everything the engine sent
	// this connection while a script ran and produced diagnostics. There is no
	// script-end event and no diagnostic on this channel, which is exactly why
	// the file above had to be watched.
	for (const FeedLine &f : c.feed())
		std::printf("  feed[%s] %s\n", f.kind == FeedKind::Local ? "L" : "E", f.text.c_str());
	std::printf("  engine lines after the subscription: %zu\n", engineLines(c).size());
	c.disconnect();
}

//! (e) The refusal, FORCED. The engine has written; the author has typed; the
//! save must not happen. With `force`, the same driver takes the other choice --
//! and the engine's tail is gone, which is what makes the refusal a fact rather
//! than a hope.
static void legLiveDirty(const Endpoint &ep, const std::string &grammar,
                         const std::string &file, int seconds, bool force)
{
	EditCore core;
	std::string err;
	check(core.open(grammar, file, err), "open the script: " + err);
	TcpClient c;
	check(c.connect(ep, err), "connect: " + err);
	c.pollFor(2000, 2);
	check(c.send("script action play filename " + file, err), "play the file: " + err);
	const bool changed = waitForWriteBack(core, seconds);
	check(changed, "the engine rewrote the file within the window");

	// The author types AFTER the run finished, into the buffer that still holds
	// the pre-run bytes. Both things now exist and only one can survive a save.
	core.moveTo(0, 0);
	// Through the editor's own two calls, not by pushing a '\n' into a line:
	// a Document line holds its terminator separately, and text with a newline
	// in it would be a line that cannot exist.
	core.insertText("# an edit made while the show was running");
	core.insertNewline();
	check(core.dirty(), "the buffer is dirty");
	check(core.diskState() == DiskState::Changed, "and the file has changed underneath it");

	if (!force) {
		const bool refused = !core.save(err);
		check(refused, "the save is REFUSED");
		std::printf("  refusal: %s\n", err.c_str());
		check(err.find("reload") != std::string::npos && err.find("save anyway") != std::string::npos,
		      "and names both choices");
	} else {
		check(core.saveOverwriting(err), "FORCED: the author's edits are written over it: " + err);
		std::printf("  forced: the engine's tail was overwritten\n");
	}
	c.disconnect();
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
	else if (leg == "live_send" && argc > 3)
		legLiveSend(ep, std::vector<std::string>(argv + 3, argv + argc));
	else if (leg == "live_get") legLiveGet(ep);
	else if (leg == "live_feed") legLiveFeed(ep, argc > 3 ? std::atoi(argv[3]) : 10);
	else if (leg == "diag") legDiag(ep);
	else if (leg == "live_diag") legLiveDiag(ep, !(argc > 3 && std::string(argv[3]) == "none"));
	else if (leg == "live_reconnect") legLiveReconnect(ep);
	else if (leg == "live_play" && argc > 4)
		legLivePlay(ep, argv[3], argv[4], argc > 5 ? std::atoi(argv[5]) : 60);
	else if (leg == "live_dirty" && argc > 4)
		legLiveDirty(ep, argv[3], argv[4], argc > 5 ? std::atoi(argv[5]) : 60, false);
	else if (leg == "live_dirty_force" && argc > 4)
		legLiveDirty(ep, argv[3], argv[4], argc > 5 ? std::atoi(argv[5]) : 60, true);
	else {
		std::fprintf(stderr, "tcpclient_test: no such leg: %s\n", leg.c_str());
		return 2;
	}
	std::printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
