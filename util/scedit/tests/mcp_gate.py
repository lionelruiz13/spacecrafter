#!/usr/bin/env python3
"""scedit's MCP server, checked by a client that is not scedit.

    tests/mcp_gate.py <scedit binary> <grammar file>

WHAT THIS PROVES. That `scedit --mcp` speaks the protocol an outside harness
will speak at it -- both eras of it -- and that the answers keep the properties
the rest of the tool is held to. In particular: a `doc` the contract file does
not have arrives as JSON **null** through the whole chain (DocIndex ->
sc_docjson -> tool result -> the wire), because the one thing this surface
exists to prevent is a model filling that gap with something plausible.

WHY PYTHON. A second implementation. A gate written in scedit's own code could
agree with a wrong reading of the specification; this one only knows what the
spec says, and the spec was fetched at implementation time (revision 2026-07-28,
plus the handshake era 2025-11-25 that deployed clients still speak).

Stdlib only, no third-party client library, and no network.
"""
import json, os, signal, subprocess, sys, threading, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fake_engine import FakeEngine, POSITION

MODERN = "2026-07-28"
LEGACY = "2025-11-25"
META_VERSION = "io.modelcontextprotocol/protocolVersion"
META_CAPS = "io.modelcontextprotocol/clientCapabilities"
META_CLIENT = "io.modelcontextprotocol/clientInfo"

failures = []
checks = 0


def check(ok, what):
    global checks
    checks += 1
    if not ok:
        failures.append(what)
    print(("  ok      " if ok else "  FAIL    ") + what)


class Server:
    """One `scedit --mcp` subprocess, one line per message."""

    def __init__(self, binary, grammar):
        self.p = subprocess.Popen([binary, "--grammar", grammar, "--mcp"],
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE, text=True, bufsize=1)

    def send_raw(self, line):
        self.p.stdin.write(line + "\n")
        self.p.stdin.flush()

    def read(self):
        line = self.p.stdout.readline()
        if not line:
            raise SystemExit("scedit --mcp closed its output unexpectedly")
        return json.loads(line)

    def request(self, method, params=None, id=None, meta=None):
        msg = {"jsonrpc": "2.0", "id": id if id is not None else next_id(), "method": method}
        p = dict(params or {})
        if meta is not None:
            p["_meta"] = meta
        if p:
            msg["params"] = p
        self.send_raw(json.dumps(msg))
        return self.read()

    def notify(self, method, params=None):
        msg = {"jsonrpc": "2.0", "method": method}
        if params:
            msg["params"] = params
        self.send_raw(json.dumps(msg))

    def close(self):
        self.p.stdin.close()
        return self.p.wait(timeout=10)


_id = [0]


def next_id():
    _id[0] += 1
    return _id[0]


def modern_meta(version=MODERN, caps=True):
    m = {META_VERSION: version, META_CLIENT: {"name": "mcp_gate", "version": "1"}}
    if caps:
        m[META_CAPS] = {}
    return m


def call(s, tool, args, meta=None):
    r = s.request("tools/call", {"name": tool, "arguments": args}, meta=meta)
    return r


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    binary, grammar = sys.argv[1], sys.argv[2]
    signal.alarm(180)   # a hung server is a failure, not a stalled gate
    s = Server(binary, grammar)

    print("A. the handshake era (2025-11-25): initialize, initialized, tools/list")
    r = s.request("initialize", {"protocolVersion": LEGACY, "capabilities": {},
                                 "clientInfo": {"name": "mcp_gate", "version": "1"}})
    res = r.get("result", {})
    check(res.get("protocolVersion") == LEGACY, "initialize answers the protocol version asked for")
    check(res.get("serverInfo", {}).get("name") == "scedit", "serverInfo names scedit")
    check(res.get("capabilities", {}).get("tools") is not None, "the tools capability is declared")
    check(bool(res.get("instructions")), "instructions are given to the model")
    check("resultType" not in res, "a handshake-era result carries no resultType (that field is 2026-07-28's)")

    s.notify("notifications/initialized")
    r = s.request("ping")
    check("result" in r, "a notification is not answered -- the next line is the ping's result")

    r = s.request("tools/list")
    tools = r["result"]["tools"]
    names = [t["name"] for t in tools]
    check(names == ["doc_lookup", "doc_search", "check_script", "run_command"],
          f"tools/list returns the four registered tools in a fixed order: {names}")
    for t in tools:
        check(bool(t.get("description")) and len(t["description"]) > 80,
              f"{t['name']}: a description written for a reader who knows nothing")
        check(t.get("inputSchema", {}).get("type") == "object",
              f"{t['name']}: inputSchema is a JSON Schema object")

    print("B. the answers, through the protocol")
    r = call(s, "doc_lookup", {"command": "dso3d", "name": "z_reflection"})
    sc = r["result"]["structuredContent"]
    check(r["result"]["isError"] is False, "doc_lookup of a known key is not an error")
    check("doc" in sc and sc["doc"] is None,
          "THE HONEST NULL SURVIVES: dso3d.z_reflection arrives as `doc: null`")
    check(sc["present"] is True and sc["notes"].startswith("DEFECT"),
          "the flagged key still carries its note")
    check(r["result"]["content"][0]["type"] == "text"
          and json.loads(r["result"]["content"][0]["text"]) == sc,
          "the text block carries the same answer, serialised")

    r = call(s, "doc_lookup", {"command": "suntrace", "name": "sun"})
    check(r["result"]["structuredContent"]["doc"] is None, "suntrace.sun arrives as `doc: null` too")

    r = call(s, "doc_lookup", {"command": "flag", "name": "stars"})
    sc = r["result"]["structuredContent"]
    check(sc["present"] is True and sc["doc"] is None,
          "a v1 family name: present true, doc null -- the name exists, the sentence does not")

    r = call(s, "doc_lookup", {"command": "set", "name": "atmosphere_fade_duration"})
    sc = r["result"]["structuredContent"]
    check(isinstance(sc["doc"], str) and sc["doc"],
          "a v2 family name answers with the documentation the doc pass wrote")

    r = call(s, "doc_lookup", {"command": "div"})
    sc = r["result"]["structuredContent"]
    check(sc["alias_of"] == "divide" and sc["doc"].startswith("Short spelling"),
          "an alias answers with its own doc and names its canonical command")
    check(sc["key_grammar"] is not None, "and with the canonical command's key grammar")

    r = call(s, "doc_lookup", {})
    cat = r["result"]["structuredContent"]
    check(cat["kind"] == "catalogue" and len(cat["commands"]) == 65,
          f"the catalogue lists every command ({len(cat['commands'])})")
    check(all("name" in c and "doc" in c for c in cat["commands"]),
          "every catalogue entry carries a name and a doc slot")
    fams = {c["name"]: c for c in cat["commands"] if c.get("family")}
    # FOUR, not the three F64's hand-written map listed: `font` names
    # families.font_targets in the contract, and reading the contract instead of
    # a copy of it is the whole point of this surface (see the journal entry).
    check(sorted(fams) == ["color", "flag", "font", "set"],
          f"every command that names a family carries its names: {sorted(fams)}")
    check(len(fams["flag"]["members"]) == 97 and len(fams["set"]["members"]) == 43
          and len(fams["color"]["members"]) == 46 and len(fams["font"]["members"]) == 10,
          "97 flag names, 43 set names, 46 colour names, 10 font targets -- the second catalogue level")
    check(all(m["doc"] is None for m in fams["flag"]["members"]),
          "the 97 flag names are honest blanks, not invented sentences")

    r = call(s, "doc_search", {"query": "play a sound in the dome", "scope": "commands", "limit": 3})
    sc = r["result"]["structuredContent"]
    check(sc["results"] and sc["results"][0]["command"] == "media",
          "doc_search ranks `media` first for a request about playing a sound")
    check(all(x["score"] > 0 for x in sc["results"]), "every returned page shares a word with the request")
    r = call(s, "doc_search", {"query": "xyzzy plugh"})
    check(r["result"]["structuredContent"]["results"] == [],
          "a request that shares no word with any page returns nothing rather than a guess")

    r = call(s, "check_script", {"text": "flag stars on\nzomo\n", "label": "probe.sts"})
    sc = r["result"]["structuredContent"]
    ids = [d["id"] for d in sc["diagnostics"]]
    check("unknown-command" in ids, f"check_script finds the unknown command: {ids}")
    d = sc["diagnostics"][0]
    check(all(k in d for k in ("file", "line", "severity", "id", "message", "span")),
          "a finding carries file, line, severity, id, message and span")
    check(d["file"] == "probe.sts", "findings are reported under the label the caller gave")
    r = call(s, "check_script", {"text": "flag stars on\n"})
    check(r["result"]["structuredContent"]["counts"]["total"] == 0,
          "a clean line yields no findings")

    print("B2. run_command, against a stand-in engine (tests/fake_engine.py)")
    # The tool opens a real socket, so the gate gives it something real to open
    # one to. What the stand-in RECEIVED is asserted as well as what came back:
    # a tool that reports a reply it invented would pass the second check alone.
    with FakeEngine() as eng:
        r = call(s, "run_command", {"command": "get status position",
                                    "host": eng.host, "port": eng.port, "wait_ms": 2000})
        sc = r["result"]["structuredContent"]
        check(r["result"]["isError"] is False, "run_command against a live engine is not an error")
        check(sc["sent"] is True and sc["endpoint"] == "%s:%d" % (eng.host, eng.port),
              "it reports what it sent and where")
        # VERBATIM includes the leading space the engine's own printf writes:
        # the client strips the record's terminators and nothing else.
        check(sc["replies"] == [POSITION],
              "the engine's answer comes back verbatim: %r" % sc["replies"])
        check(sc["reply_count"] == 1, "with its count")
        check("$LOGON" in eng.lines() and "get status position" in eng.lines(),
              "the stand-in saw the subscription and the command: %s" % eng.lines())
        # WAITED for, not sampled: disconnect now writes TWO unsubscribe lines
        # and closes, so reading the transcript the instant the call returns is
        # a race the widened window made visible (it went red here once, on a
        # run that had passed a minute earlier). `wait_for` is this file's own
        # rule - a gate never sleeps a fixed time waiting for the other side.
        check(eng.wait_for("$LOGOFF", timeout=5),
              "and the connection was closed politely -- one call, one connection")
        check("$DIAGOFF" in eng.lines(),
              "unsubscribing from the diagnostic link too: %s"
              % [l for l in eng.lines() if l.startswith("$")])
        check("$LOGON" in sc["note"] and "OTHER clients" in sc["note"],
              "the note warns that a reply may be another client's: %s" % sc["note"][:80])

        # A command the engine answers with silence -- which is MOST of them. The
        # tool must not dress that up as either outcome.
        r = call(s, "run_command", {"command": "flag stars on",
                                    "host": eng.host, "port": eng.port, "wait_ms": 300})
        sc = r["result"]["structuredContent"]
        check(r["result"]["isError"] is False and sc["sent"] is True,
              "an ordinary command is sent successfully")
        check(sc["replies"] == [], "and answered with nothing")
        check(sc["diagnostics"] == [] and sc["diagnostic_count"] == 0,
              "with no diagnostic either")
        # The note CHANGED with INTENT 11.188 and this assertion changed with
        # it, deliberately: silence used to mean nothing at all, and now it
        # means "not refused at the top level" and no more. The three things
        # that still make silence uninformative are named in the note, and the
        # gate pins all three - a note that dropped one would be a note that
        # over-promises.
        check("not refused at the top level" in sc["note"]
              and "success is still SILENT" in sc["note"]
              and "inside another command" in sc["note"]
              and "OLDER engine" in sc["note"],
              "and the note says exactly how much that silence is worth: %s" % sc["note"][:120])
        check("flag stars on" in eng.lines(), "the stand-in did receive it")

        # A REFUSAL comes back, split into fields, and says whose it was.
        def refuse():
            if eng.wait_for("flagg stars on", timeout=20):
                eng.diag_broadcast("tcp#9", "Unrecognized or malformed command name",
                                   "flagg stars on")
        t = threading.Thread(target=refuse, daemon=True)
        t.start()
        r = call(s, "run_command", {"command": "flagg stars on",
                                    "host": eng.host, "port": eng.port, "wait_ms": 2500})
        t.join(timeout=25)
        sc = r["result"]["structuredContent"]
        check(sc["diagnostic_count"] == 1, "a refused command comes back as ONE diagnostic")
        check(sc["diagnostics"] and sc["diagnostics"][0]["origin"] == "tcp#9"
              and sc["diagnostics"][0]["subject"] == "flagg stars on"
              and sc["diagnostics"][0]["message"] == "Unrecognized or malformed command name",
              "split into origin / message / subject: %s" % sc["diagnostics"][:1])
        check(sc["diagnostics"] and sc["diagnostics"][0]["raw"].startswith("$DIAG|"),
              "with the raw record kept, so nothing is lost in the split")
        check(sc["replies"] == [], "and it is NOT in `replies`: the two lists are exclusive")
        check("REFUSED" in sc["note"] and "origin" in sc["note"],
              "the note points at the diagnostics and warns about the origin: %s" % sc["note"][:100])
        check("$DIAGON" in eng.lines(),
              "the tool's own connection subscribed to the diagnostic link: %s"
              % [l for l in eng.lines() if l.startswith("$")])

    # NOW nothing is listening: the same call must fail with a sentence the
    # caller can act on, not a crash and not a fake success. The port is one
    # that was bound and released, so the refusal is the operating system's.
    import socket as _socket
    _s = _socket.socket(); _s.bind(("127.0.0.1", 0)); dead_port = _s.getsockname()[1]; _s.close()
    r = call(s, "run_command", {"command": "flag stars on",
                                "host": "127.0.0.1", "port": dead_port, "wait_ms": 300})
    sc = r["result"]["structuredContent"]
    check(r["result"]["isError"] is True, "with no engine listening, run_command is a tool error")
    check(sc["sent"] is False and sc["error"] == "no-engine",
          "it says plainly that nothing was sent")
    check("io:enable_tcp" in sc["message"],
          "and names what to check on the engine: %s" % sc["message"][:120])
    check(sc["replies"] == [], "with no replies invented")

    print("C. what must be refused")
    r = call(s, "doc_lookup", {"command": "zomo"})
    sc = r["result"]["structuredContent"]
    check(r["result"]["isError"] is True, "an unknown command is a tool error the caller can correct")
    check(sc["error"] == "unknown-command" and sc["did_you_mean"] == "zoom"
          and "families.commands" in sc["vocabulary"],
          "and the answer names the vocabulary and the nearest name in it")
    r = call(s, "doc_lookup", {"name": "stars"})
    check(r["result"]["isError"] is True, "a name without a command is refused, not guessed")
    r = call(s, "doc_search", {})
    check(r["result"]["isError"] is True, "doc_search without a query is refused")
    r = call(s, "check_script", {"text": "flag stars on", "path": "/tmp/x.sts"})
    check(r["result"]["isError"] is True, "check_script refuses both text and path at once")
    r = call(s, "check_script", {"path": "/nonexistent/f66.sts"})
    check(r["result"]["isError"] is True, "an unreadable path is a tool error, not a crash")
    r = call(s, "run_command", {})
    check(r["result"]["isError"] is True, "run_command without a command is refused")
    r = call(s, "run_command", {"command": "flag stars on\nflag planets on"})
    check(r["result"]["isError"] is True and "ONE line" in r["result"]["structuredContent"]["message"],
          "a two-line command is refused before any socket is opened")
    r = call(s, "run_command", {"command": "flag stars on", "port": 0})
    check(r["result"]["isError"] is True, "an impossible port is refused")
    r = call(s, "run_command", {"command": "flag stars on", "wait_ms": 1})
    check(r["result"]["isError"] is True,
          "a wait too short to mean anything is refused rather than silently raised")

    r = s.request("tools/call", {"name": "no_such_tool", "arguments": {}})
    check(r.get("error", {}).get("code") == -32602 and "no_such_tool" in r["error"]["message"],
          "an unknown TOOL is a protocol error (-32602), not a tool result")
    r = s.request("no/such/method")
    check(r.get("error", {}).get("code") == -32601, "an unknown method is -32601")
    s.send_raw('{"jsonrpc": "2.0", "id": 99, "method": ')
    r = s.read()
    check(r.get("error", {}).get("code") == -32700 and r.get("id") is None,
          "a malformed line is a parse error (-32700) with a null id")
    s.send_raw('{"jsonrpc": "2.0", "id": 98}')
    r = s.read()
    check(r.get("error", {}).get("code") == -32600 and r.get("id") == 98,
          "a request with no method is an invalid request (-32600), answered under its own id")
    s.send_raw('[{"jsonrpc": "2.0", "id": 97, "method": "ping"}]')
    r = s.read()
    check(r.get("error", {}).get("code") == -32600, "a batch is refused rather than half-answered")

    print("D. the stateless era (2026-07-28): per-request metadata, server/discover")
    r = s.request("server/discover", meta=modern_meta())
    res = r["result"]
    check(res.get("resultType") == "complete", "a modern result carries resultType complete")
    check(MODERN in res["supportedVersions"], f"server/discover lists {MODERN}")
    check(res["_meta"]["io.modelcontextprotocol/serverInfo"]["name"] == "scedit",
          "the server identifies itself in the result's _meta")
    r = s.request("server/discover")
    check(r.get("error", {}).get("code") == -32602,
          "server/discover without the required _meta is invalid params (-32602)")
    r = s.request("tools/list", meta=modern_meta(version="1900-01-01"))
    check(r.get("error", {}).get("code") == -32022
          and r["error"]["data"]["supported"] == [MODERN]
          and r["error"]["data"]["requested"] == "1900-01-01",
          "an unsupported protocol version is -32022, listing what is supported")
    r = s.request("tools/list", meta=modern_meta(caps=False))
    check(r.get("error", {}).get("code") == -32602,
          "a modern request missing clientCapabilities is -32602, as the spec prescribes")
    r = call(s, "run_command", {"command": "flag stars on", "port": 1},
             meta=modern_meta())
    check(r["result"]["resultType"] == "complete" and r["result"]["isError"] is True,
          "the live tool answers in the stateless era too, refusal and all")
    r = call(s, "doc_lookup", {"command": "flag", "name": "stars"}, meta=modern_meta())
    check(r["result"]["resultType"] == "complete"
          and r["result"]["structuredContent"]["doc"] is None,
          "the same answer, in the stateless era")
    r = s.request("initialize", {"protocolVersion": LEGACY}, meta=modern_meta())
    check(r.get("error", {}).get("code") == -32601,
          "a modern request may not ask for the handshake it replaced")

    rc = s.close()
    check(rc == 0, f"the server exits 0 when its input closes (got {rc})")
    err = s.p.stderr.read()
    check("MCP server on stdio" in err, "the server's own logging went to stderr, never to stdout")

    print(f"{checks} checks, {len(failures)} failures")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
