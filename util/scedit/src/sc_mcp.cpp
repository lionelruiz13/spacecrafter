/*
 * scedit — sc_mcp.cpp
 *
 * The adapter: JSON-RPC 2.0 over stdio, the two protocol eras, and the tool
 * registry. Protocol shapes are the fetched specification's (see sc_mcp.hpp for
 * the revision and the fetch date); the answers are sc_docjson's.
 */

#include "sc_mcp.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#include "sc_check.hpp"
#include "sc_docjson.hpp"

namespace scedit {

using nlohmann::json;

namespace {

//! scedit had no version string before this surface existed; a client displays
//! one, so here it is, named after the fifth slice of main.cpp's header.
const char *const kServerVersion = "0.5.0";
const char *const kModernVersion = "2026-07-28";
const char *const kLegacyVersion = "2025-11-25";

const char *const kMetaProtocol = "io.modelcontextprotocol/protocolVersion";
const char *const kMetaClientCaps = "io.modelcontextprotocol/clientCapabilities";
const char *const kMetaServerInfo = "io.modelcontextprotocol/serverInfo";

//! What the outside model is told this server is for, once, at handshake time.
const char *const kInstructions =
	"scedit answers questions about the spacecrafter planetarium's script language from the "
	"engine's own machine-readable command contract, and checks scripts the way the engine "
	"reads them. Use doc_search to find which page answers a request, doc_lookup to read a "
	"command or one of its keys, and check_script before showing a script to anyone: it reports "
	"exactly where the engine's reading will differ from what the text plainly means. A `doc` "
	"field that is null means no documentation has been extracted for that name yet — say so "
	"rather than filling the gap.";

json serverInfo()
{
	json j;
	j["name"] = "scedit";
	j["title"] = "spacecrafter script documentation and checker";
	j["version"] = kServerVersion;
	return j;
}

json toolCapabilities()
{
	json caps;
	// The tool set is fixed at build time: there is nothing to notify about.
	caps["tools"] = json::object({{"listChanged", false}});
	return caps;
}

// ------------------------------------------------------------ the tool handlers

std::string argString(const json &args, const char *key, bool &present)
{
	present = args.is_object() && args.contains(key) && args.at(key).is_string();
	return present ? args.at(key).get<std::string>() : std::string();
}

ToolResult toolError(const std::string &message, const json &extra = json::object())
{
	ToolResult r;
	r.is_error = true;
	r.structured = extra;
	r.structured["error"] = "invalid-arguments";
	r.structured["message"] = message;
	return r;
}

ToolResult docLookupTool(const ToolContext &ctx, const json &args)
{
	bool hasCommand = false, hasName = false;
	const std::string command = argString(args, "command", hasCommand);
	const std::string name = argString(args, "name", hasName);
	ToolResult r;
	if (!hasCommand) {
		if (hasName)
			return toolError("`name` names a key or a family name of a command, so `command` "
			                 "must be given with it; call with no arguments for the catalogue");
		r.structured = docCatalogue(ctx.grammar, ctx.docs);
		return r;
	}
	const DocAnswer a = docLookup(ctx.grammar, ctx.docs, command, name);
	r.structured = a.value;
	r.is_error = !a.found;   // a name that is not in the vocabulary is the
	                         // caller's to correct, and the answer says from
	                         // which vocabulary it is missing
	return r;
}

ToolResult docSearchTool(const ToolContext &ctx, const json &args)
{
	bool hasQuery = false;
	const std::string query = argString(args, "query", hasQuery);
	if (!hasQuery || query.empty())
		return toolError("`query` is required: the words to search the documentation for");
	SearchScope scope = SearchScope::All;
	bool hasScope = false;
	const std::string s = argString(args, "scope", hasScope);
	if (hasScope) {
		if (s == "commands") scope = SearchScope::Commands;
		else if (s != "all")
			return toolError("`scope` is `all` (commands, keys and family names) or `commands`");
	}
	std::size_t limit = 10;
	if (args.is_object() && args.contains("limit") && args.at("limit").is_number_unsigned())
		limit = args.at("limit").get<std::size_t>();
	ToolResult r;
	r.structured = docSearch(ctx.grammar, ctx.docs, query, scope, limit);
	return r;
}

ToolResult checkScriptTool(const ToolContext &ctx, const json &args)
{
	bool hasText = false, hasPath = false, hasLabel = false;
	const std::string text = argString(args, "text", hasText);
	const std::string path = argString(args, "path", hasPath);
	const std::string label = argString(args, "label", hasLabel);
	if (hasText == hasPath)
		return toolError("give either `text` (the script's lines) or `path` (a file to read), "
		                 "not both and not neither");
	std::vector<Diagnostic> diags;
	std::string shown = hasLabel ? label : (hasPath ? path : std::string("<text>"));
	if (hasPath) {
		std::string ioErr;
		diags = checkFile(ctx.grammar, path, ioErr);
		if (!ioErr.empty())
			return toolError(ioErr, json::object({{"path", path}}));
	} else {
		diags = checkBuffer(ctx.grammar, shown, text);
	}
	ToolResult r;
	r.structured = diagnosticsJson({shown}, diags);
	return r;
}

//! The tool that touches the world. Everything else in this server reads a
//! file; this one opens a socket to a running planetarium and makes it do
//! something.
ToolResult runCommandTool(const ToolContext &ctx, const json &args)
{
	bool hasCommand = false;
	const std::string command = argString(args, "command", hasCommand);
	if (!hasCommand || command.empty())
		return toolError("`command` is required: one line of the spacecrafter script "
		                 "language, e.g. `flag stars on`");
	if (command.find('\n') != std::string::npos || command.find('\r') != std::string::npos)
		return toolError("`command` is ONE line: this text carries a line break. Send the "
		                 "lines one call at a time, so that you know which of them ran.");

	Endpoint ep = ctx.live_endpoint;
	bool hasHost = false;
	const std::string host = argString(args, "host", hasHost);
	if (hasHost) {
		if (host.empty())
			return toolError("`host` is empty; omit it to use " + ctx.live_endpoint.host);
		ep.host = host;
	}
	if (args.is_object() && args.contains("port")) {
		const json &p = args.at("port");
		if (!p.is_number_integer() || p.get<long long>() < 1 || p.get<long long>() > 65535)
			return toolError("`port` is a TCP port between 1 and 65535");
		ep.port = (int)p.get<long long>();
	}
	// The wait is bounded on BOTH sides: a caller cannot ask for zero (and then
	// report a reply as absent when it simply had not arrived) and cannot hold
	// the server for a minute (this is a stdio server answering one line at a
	// time).
	long waitMs = 1500;
	if (args.is_object() && args.contains("wait_ms")) {
		const json &w = args.at("wait_ms");
		if (!w.is_number_integer() || w.get<long long>() < 100 || w.get<long long>() > 10000)
			return toolError("`wait_ms` is how long to wait for a reply, between 100 and 10000");
		waitMs = (long)w.get<long long>();
	}

	json out;
	out["command"] = command;
	out["endpoint"] = ep.text();
	out["waited_ms"] = waitMs;

	TcpClient client;
	std::string err;
	if (!client.connect(ep, err)) {
		ToolResult r;
		r.is_error = true;
		out["sent"] = false;
		out["error"] = "no-engine";
		out["message"] = err;
		out["replies"] = json::array();
		r.structured = out;
		return r;
	}
	// The subscription confirmation is the engine's answer to $LOGON, not to
	// this command; it is read and dropped so that `replies` holds what the
	// command produced and nothing else.
	client.pollFor(500, 1);
	client.clearFeed();

	if (!client.send(command, err)) {
		client.disconnect();
		ToolResult r;
		r.is_error = true;
		out["sent"] = false;
		out["error"] = "not-sent";
		out["message"] = err;
		out["replies"] = json::array();
		r.structured = out;
		return r;
	}
	client.pollFor((int)waitMs);
	json replies = json::array();
	for (const FeedLine &f : client.feed())
		if (f.kind == FeedKind::Engine)
			replies.push_back(f.text);
	client.disconnect();

	out["sent"] = true;
	out["replies"] = replies;
	out["reply_count"] = replies.size();
	// The single most important field for a model reading this: what an empty
	// reply list means. Saying it once, here, is cheaper than a model guessing
	// it every time.
	out["note"] = replies.empty()
	                      ? "The engine sent nothing back. That is the normal case: only "
	                        "`get status ...` and `search name ...` produce a reply, and every "
	                        "other command runs in silence. This is NOT evidence that the "
	                        "command succeeded, and NOT evidence that it failed — the engine "
	                        "writes its refusals to its own log file, which is not on this "
	                        "channel. Read a state back with `get status ...` if you need to "
	                        "know what happened."
	                      : "Replies to this connection, and anything the engine broadcast to "
	                        "its $LOGON feed while we waited — which includes answers to OTHER "
	                        "clients' commands.";
	ToolResult r;
	r.structured = out;
	return r;
}

// ------------------------------------------------------------- the registry

std::vector<Tool> buildRegistry()
{
	std::vector<Tool> tools;

	Tool lookup;
	lookup.name = "doc_lookup";
	lookup.title = "Read a command's documentation page";
	lookup.description =
		"Read what the spacecrafter engine's own command contract says about one command of its "
		"script language, or about one argument key or family name of that command. Call with no "
		"arguments to get the catalogue: every command with its one-line description, and under "
		"the three commands that name a family (flag, set, color) the names that family accepts. "
		"Fields are the contract's own text: `doc` (a null `doc` means no documentation has been "
		"extracted for that name — report the gap, never invent one), `value_domain` and `values` "
		"(what may be written as the value), `default` (what happens when the argument is "
		"omitted), `required`, `source` (the engine file and lines the fact was read from), and "
		"`notes`. An unknown name is answered, not guessed at: the reply names the vocabulary the "
		"name is missing from and the nearest name in it.";
	lookup.input_schema = json::parse(R"({
		"type": "object",
		"properties": {
			"command": {"type": "string", "description": "A script command, e.g. `flag`, `image`, `moveto`. Omit for the whole catalogue."},
			"name": {"type": "string", "description": "An argument key of that command (`image filename`) or a name of the family it draws its keys from (`flag stars`). Omit for the command's own page."}
		},
		"additionalProperties": false
	})");
	lookup.handler = docLookupTool;
	tools.push_back(lookup);

	Tool search;
	search.name = "doc_search";
	search.title = "Find which documentation pages a request is about";
	search.description =
		"Rank the documentation pages of the spacecrafter script language against a few words of "
		"a request (\"draw the constellation lines\", \"play a sound\"), best first. A page is a "
		"command, an argument key of a command, or a name of a command's family; each result "
		"carries `page`, which is exactly what to pass to doc_lookup. The ranking is a stated "
		"word-overlap score over page names and documentation lines — no inference and no "
		"synonyms, so a request that shares no word with any page returns nothing rather than a "
		"plausible-looking guess.";
	search.input_schema = json::parse(R"({
		"type": "object",
		"properties": {
			"query": {"type": "string", "description": "The words to look for, in any order."},
			"scope": {"type": "string", "enum": ["all", "commands"], "description": "`all` (default) ranks commands, argument keys and family names; `commands` ranks commands only."},
			"limit": {"type": "integer", "minimum": 1, "description": "How many results to return (default 10)."}
		},
		"required": ["query"],
		"additionalProperties": false
	})");
	search.handler = docSearchTool;
	tools.push_back(search);

	Tool check;
	check.name = "check_script";
	check.title = "Check a spacecrafter script the way the engine reads it";
	check.description =
		"Analyse a spacecrafter script and report every place where the engine's reading will "
		"differ from what the text plainly means: unknown commands and keys, a key with no value "
		"(the engine drops it silently), duplicated keys, bytes that look like a space and are "
		"not, unclosed blocks. Each finding carries the line, a severity, an identifier and the "
		"byte range it is about. Every rule reports engine BEHAVIOUR, never style, and rules that "
		"cannot be checked without false positives are not run at all — so a finding is a fact "
		"about what will happen, and an empty result is not a promise that the script does what "
		"its author wanted. Run this on any script before offering it to a user.";
	check.input_schema = json::parse(R"({
		"type": "object",
		"properties": {
			"text": {"type": "string", "description": "The script's lines. Give this or `path`."},
			"path": {"type": "string", "description": "A script file to read from disk. Give this or `text`."},
			"label": {"type": "string", "description": "The name to report findings under when `text` is given (default `<text>`)."}
		},
		"additionalProperties": false
	})");
	check.handler = checkScriptTool;
	tools.push_back(check);

	Tool run;
	run.name = "run_command";
	run.title = "Run one command on a live spacecrafter";
	run.description =
		"Send ONE line of the spacecrafter script language to a RUNNING planetarium engine over "
		"its control socket, and return whatever it says back. This is not a simulation and not "
		"a dry run: the dome moves, the show changes, and there is no undo. Ask the person you "
		"are working for before using it, and check the line with check_script first. "
		"WHAT COMES BACK: only `get status <what>` and `search name <name>` produce a reply. "
		"Every other command runs in SILENCE, so an empty `replies` means neither success nor "
		"failure — the engine writes its refusals to a log file that is not on this channel. To "
		"find out what actually happened, read a state back with `get status ...`. The `replies` "
		"list can also carry answers to OTHER clients' commands: this connection subscribes to "
		"the engine's feedback channel while it waits. A command that plays a script returns as "
		"soon as the script STARTS; nothing announces that it has ended.";
	run.input_schema = json::parse(R"({
		"type": "object",
		"properties": {
			"command": {"type": "string", "description": "One command line, exactly as it would be written in a script, e.g. `flag stars on` or `get status position`."},
			"host": {"type": "string", "description": "Where the engine is. Default 127.0.0.1 (or whatever --tcp named when this server was started)."},
			"port": {"type": "integer", "minimum": 1, "maximum": 65535, "description": "The engine's control port. Default 7805, the shipped `io:tcp_port_in`."},
			"wait_ms": {"type": "integer", "minimum": 100, "maximum": 10000, "description": "How long to wait for a reply before answering, in milliseconds (default 1500). Waiting longer does not make a silent command speak."}
		},
		"required": ["command"],
		"additionalProperties": false
	})");
	run.handler = runCommandTool;
	tools.push_back(run);

	return tools;
}

// ----------------------------------------------------------------- the wire

void writeMessage(const json &msg)
{
	// One message, one line, no embedded newline: the stdio binding's whole
	// framing rule. `dump()` without indentation cannot produce one.
	const std::string s = msg.dump();
	std::fwrite(s.data(), 1, s.size(), stdout);
	std::fputc('\n', stdout);
	std::fflush(stdout);
}

json errorResponse(const json &id, int code, const std::string &message,
                   const json &data = json())
{
	json e;
	e["code"] = code;
	e["message"] = message;
	if (!data.is_null())
		e["data"] = data;
	json r;
	r["jsonrpc"] = "2.0";
	r["id"] = id.is_null() ? json(nullptr) : id;
	r["error"] = e;
	return r;
}

json resultResponse(const json &id, json result, bool modern)
{
	// 2026-07-28 requires `resultType` on every result; the legacy revision has
	// no such field, and its clients are told to treat an absent one as
	// "complete". Sending it only to the era that defines it keeps each answer
	// valid against the schema the client is actually holding.
	if (modern)
		result["resultType"] = "complete";
	json meta = result.contains("_meta") ? result.at("_meta") : json::object();
	meta[kMetaServerInfo] = serverInfo();
	result["_meta"] = meta;
	json r;
	r["jsonrpc"] = "2.0";
	r["id"] = id;
	r["result"] = result;
	return r;
}

json toolListJson()
{
	json tools = json::array();
	for (const Tool &t : registeredTools()) {
		json j;
		j["name"] = t.name;
		j["title"] = t.title;
		j["description"] = t.description;
		j["inputSchema"] = t.input_schema;
		tools.push_back(j);
	}
	json r;
	r["tools"] = tools;
	return r;
}

const Tool *findTool(const std::string &name)
{
	for (const Tool &t : registeredTools())
		if (t.name == name)
			return &t;
	return nullptr;
}

json callToolJson(const ToolContext &ctx, const Tool &t, const json &args)
{
	const ToolResult res = t.handler(ctx, args);
	json content = json::array();
	json text;
	text["type"] = "text";
	text["text"] = res.structured.dump(2);
	content.push_back(text);
	json r;
	r["content"] = content;
	r["structuredContent"] = res.structured;
	r["isError"] = res.is_error;
	return r;
}

//! The era of ONE request: modern when it carries the per-request protocol
//! version the 2026-07-28 revision requires, legacy otherwise. The connection
//! itself has no era — the spec's statelessness rule is that a server must not
//! infer anything from an earlier request.
struct Era {
	bool modern = false;
	std::string version;
};

Era eraOf(const json &params)
{
	Era e;
	if (params.is_object() && params.contains("_meta") && params.at("_meta").is_object()) {
		const json &m = params.at("_meta");
		if (m.contains(kMetaProtocol) && m.at(kMetaProtocol).is_string()) {
			e.modern = true;
			e.version = m.at(kMetaProtocol).get<std::string>();
		}
	}
	return e;
}

//! Returns false and fills `err` when a modern request is not answerable as
//! sent: the version is one we do not implement (-32022, listing what we do),
//! or a required `_meta` field is missing (-32602, as the spec prescribes).
bool modernRequestOk(const json &id, const json &params, const Era &era, json &err)
{
	if (era.version != kModernVersion) {
		json data;
		data["supported"] = json::array({kModernVersion});
		data["requested"] = era.version;
		err = errorResponse(id, -32022, "Unsupported protocol version", data);
		return false;
	}
	const json &m = params.at("_meta");
	if (!m.contains(kMetaClientCaps)) {
		err = errorResponse(id, -32602,
		                    std::string("Invalid params: _meta is missing the required field `") +
		                    kMetaClientCaps + "`");
		return false;
	}
	return true;
}

json legacyInitialize(const json &id, const json &params)
{
	std::string asked;
	if (params.is_object() && params.contains("protocolVersion") && params.at("protocolVersion").is_string())
		asked = params.at("protocolVersion").get<std::string>();
	json r;
	// The revision this server was written against is the one it answers with;
	// any other request gets our version and the client decides whether it can
	// speak it, which is what the 2025-11-25 lifecycle prescribes.
	r["protocolVersion"] = asked == kLegacyVersion ? asked : std::string(kLegacyVersion);
	r["capabilities"] = toolCapabilities();
	r["serverInfo"] = serverInfo();
	r["instructions"] = kInstructions;
	json out;
	out["jsonrpc"] = "2.0";
	out["id"] = id;
	out["result"] = r;
	return out;
}

json discoverResult()
{
	json r;
	r["supportedVersions"] = json::array({kModernVersion, kLegacyVersion});
	r["capabilities"] = toolCapabilities();
	r["instructions"] = kInstructions;
	return r;
}

} // namespace

const std::vector<Tool> &registeredTools()
{
	static const std::vector<Tool> tools = buildRegistry();
	return tools;
}

int runMcpServer(const std::string &grammarPath, const Endpoint &liveEndpoint)
{
	Grammar g;
	DocIndex d;
	std::string err;
	if (!g.load(grammarPath, err) || !d.load(grammarPath, err)) {
		std::fprintf(stderr, "scedit: %s\n", err.c_str());
		return 2;
	}
	const ToolContext ctx{g, d, grammarPath, liveEndpoint};
	std::fprintf(stderr, "scedit: MCP server on stdio, %zu tools, contract %s\n",
	             registeredTools().size(), grammarPath.c_str());

	std::string line;
	while (std::getline(std::cin, line)) {
		if (line.find_first_not_of(" \t\r\n") == std::string::npos)
			continue;
		json msg;
		try {
			msg = json::parse(line);
		} catch (const std::exception &e) {
			writeMessage(errorResponse(json(nullptr), -32700, std::string("Parse error: ") + e.what()));
			continue;
		}
		if (!msg.is_object() || !msg.contains("method") || !msg.at("method").is_string()) {
			// A batch (a JSON array) lands here too: this server answers one
			// message per line, and says so rather than half-answering.
			writeMessage(errorResponse(msg.is_object() && msg.contains("id") ? msg.at("id") : json(nullptr),
			                           -32600, "Invalid Request: expected a JSON-RPC object with a string `method`"));
			continue;
		}
		const std::string method = msg.at("method").get<std::string>();
		const json params = msg.contains("params") ? msg.at("params") : json::object();

		if (!msg.contains("id")) {
			// A notification. Nothing this server does depends on one, and a
			// response to one would itself be a protocol violation.
			continue;
		}
		const json id = msg.at("id");
		const Era era = eraOf(params);
		if (era.modern) {
			json e;
			if (!modernRequestOk(id, params, era, e)) {
				writeMessage(e);
				continue;
			}
		}

		if (method == "initialize") {
			// Legacy era only: a modern client has no handshake to perform.
			writeMessage(era.modern
			                     ? errorResponse(id, -32601,
			                                     "Method not found: `initialize` belongs to the "
			                                     "handshake-based protocol eras; this request "
			                                     "carries per-request metadata, so use "
			                                     "`server/discover`")
			                     : legacyInitialize(id, params));
		} else if (method == "server/discover") {
			if (!era.modern) {
				writeMessage(errorResponse(id, -32602,
				                           std::string("Invalid params: `server/discover` requires "
				                                       "_meta.`") + kMetaProtocol + "`"));
				continue;
			}
			writeMessage(resultResponse(id, discoverResult(), true));
		} else if (method == "ping") {
			writeMessage(resultResponse(id, json::object(), era.modern));
		} else if (method == "tools/list") {
			writeMessage(resultResponse(id, toolListJson(), era.modern));
		} else if (method == "tools/call") {
			std::string name;
			if (params.is_object() && params.contains("name") && params.at("name").is_string())
				name = params.at("name").get<std::string>();
			const Tool *t = findTool(name);
			if (!t) {
				writeMessage(errorResponse(id, -32602, "Unknown tool: " + name));
				continue;
			}
			const json args = params.contains("arguments") ? params.at("arguments") : json::object();
			writeMessage(resultResponse(id, callToolJson(ctx, *t, args), era.modern));
		} else {
			writeMessage(errorResponse(id, -32601, "Method not found: " + method));
		}
	}
	return 0;
}

} // namespace scedit
