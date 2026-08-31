/*
 * scedit — sc_mcp.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * scedit as a Model Context Protocol server on stdio, so that any harness with
 * an MCP client — Claude Code, an ollama-backed one, a dome operator's own —
 * can ask the CONTRACT FILE what a spacecrafter command means and can have a
 * script checked, instead of recalling a script language it half-knows. It is
 * an adapter and nothing more: every answer comes from sc_docjson.hpp, which
 * reads Grammar and DocIndex, which read grammar/sc-grammar.json. There is no
 * model call anywhere in scedit and no network of any kind.
 *
 * THE SPECIFICATION IS FETCHED, NEVER RECALLED
 * ============================================
 * Implemented against https://modelcontextprotocol.io/specification/, whose own
 * "latest" pointer resolved to revision **2026-07-28** when this was written
 * (fetched 2026-08-31). That revision has NO initialize handshake: it is
 * stateless, every request carries its protocol version in
 * `_meta["io.modelcontextprotocol/protocolVersion"]`, and a server MUST answer
 * `server/discover`. The handshake-based era it calls "legacy" (`initialize` +
 * `notifications/initialized`, revision 2025-11-25 and earlier) is what
 * deployed clients still speak. This server is therefore DUAL-ERA, which is the
 * spec's own name for the case: a request carrying the modern `_meta` is served
 * statelessly per 2026-07-28, anything else is served per 2025-11-25. What is
 * deliberately NOT implemented is listed in README.md § For machines, because a
 * silent omission is indistinguishable from a bug.
 *
 * THE REGISTRY IS THE SEAM
 * ========================
 * A tool is DECLARED in exactly one place — `registeredTools()` — as a name, a
 * description written to the zero-knowledge bar (it is what an outside model
 * reads before deciding to call it), an input schema and a handler. The
 * protocol code below names no tool and knows no tool's arguments: adding
 * `run_command` over the live engine's TCP line (dispatch task F67) is one
 * entry in that vector plus one field on ToolContext, and touches nothing here.
 *
 * STDOUT IS THE PROTOCOL. Every diagnostic, every load failure and every
 * unparseable line goes to stderr; stdout carries newline-delimited JSON-RPC
 * and nothing else, which is what the stdio binding requires.
 */

#ifndef SCEDIT_SC_MCP_HPP
#define SCEDIT_SC_MCP_HPP

#include <functional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "sc_docindex.hpp"
#include "sc_grammar.hpp"

namespace scedit {

//! What a tool handler is given. One loaded contract, shared by every call:
//! the server keeps no other state between calls (the protocol is stateless,
//! and a doc lookup has nothing to remember).
struct ToolContext {
	const Grammar &grammar;
	const DocIndex &docs;
	std::string grammar_path;
};

//! What a tool handler returns. `structured` is the answer as data (the
//! `structuredContent` of the result) and is also serialised into the text
//! block, because a client that ignores structured content must still see it.
//! `is_error` marks a TOOL EXECUTION error — a wrong argument, a file that
//! cannot be read — which the calling model is expected to read and correct;
//! protocol-level failures never come from here.
struct ToolResult {
	nlohmann::json structured;
	bool is_error = false;
};

struct Tool {
	std::string name;
	std::string title;
	std::string description;
	nlohmann::json input_schema;
	std::function<ToolResult(const ToolContext &, const nlohmann::json &args)> handler;
};

//! THE one place a tool is declared. F67 appends here.
const std::vector<Tool> &registeredTools();

//! Run the server on stdin/stdout until end of input. Returns a process exit
//! code: 0 for a clean end of stream, 2 when the contract file cannot be read.
int runMcpServer(const std::string &grammarPath);

} // namespace scedit

#endif // SCEDIT_SC_MCP_HPP
