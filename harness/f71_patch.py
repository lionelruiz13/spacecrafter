#!/usr/bin/env python3
"""Format-preserving JSON field inserter (F71).

WHY THIS EXISTS.  The scedit grammar lives in two places on purpose:
`util/scedit/grammar/sc-grammar.json` (the merged contract the tool reads) and
`util/scedit/grammar/args/unit-{1..4}.json` (the four extraction fragments that
stay in the tree as the granular source).  `scedit`'s own validator closes that
I2 exposure by comparing them: `checkFragments` in `util/scedit/src/main.cpp`
asserts `mergedArgs == fragArgs` for every command.  So any field added to one
must be added to the other, or the seed gate fails.

The merged file round-trips byte-identically through `json.dumps(indent=2)`;
the four fragments do NOT -- they are hand-formatted, with some argument specs
written on a single line and others expanded, and re-serialising them would
produce a ~13 KB reformatting diff per file that buries the real change and
throws away the author's layout.  So this module edits the TEXT, inserting a
new field immediately after an existing one and matching the surrounding
object's own layout (single-line objects get `, "f": v`; expanded objects get
a new line at the same indent).

The insertion is verified by parsing the result and comparing against the
expected value, so a botched edit fails loudly rather than silently.
"""

import json


def _skip_ws(s, i):
	while i < len(s) and s[i] in " \t\r\n":
		i += 1
	return i


def _scan_string(s, i):
	"""i points at the opening quote; return index just past the closing quote."""
	assert s[i] == '"', (i, s[i - 20:i + 20])
	i += 1
	while True:
		c = s[i]
		if c == '\\':
			i += 2
			continue
		if c == '"':
			return i + 1
		i += 1


def _scan_value(s, i):
	"""i points at the first char of a JSON value; return index just past it."""
	i = _skip_ws(s, i)
	c = s[i]
	if c == '"':
		return _scan_string(s, i)
	if c in '{[':
		close = '}' if c == '{' else ']'
		depth = 0
		while True:
			c = s[i]
			if c == '"':
				i = _scan_string(s, i)
				continue
			if c in '{[':
				depth += 1
			elif c in '}]':
				depth -= 1
				if depth == 0:
					return i + 1
			i += 1
	# number, true, false, null
	j = i
	while j < len(s) and s[j] not in ',}] \t\r\n':
		j += 1
	return j


def find_member(s, obj_start, name):
	"""Find member `name` directly inside the object starting at obj_start.

	Returns (key_start, value_start, value_end) or None.  Only depth-1 members
	are considered, so a nested object carrying the same key is not matched.
	"""
	assert s[obj_start] == '{'
	i = obj_start + 1
	while True:
		i = _skip_ws(s, i)
		if s[i] == '}':
			return None
		assert s[i] == '"', (i, s[i - 40:i + 40])
		key_start = i
		key_end = _scan_string(s, i)
		key = json.loads(s[key_start:key_end])
		i = _skip_ws(s, key_end)
		assert s[i] == ':'
		value_start = _skip_ws(s, i + 1)
		value_end = _scan_value(s, value_start)
		if key == name:
			return (key_start, value_start, value_end)
		i = _skip_ws(s, value_end)
		if s[i] == ',':
			i += 1


def find_object(s, path):
	"""Walk a list of member names from the document root; return the offset of
	the '{' that opens the object at that path."""
	i = _skip_ws(s, 0)
	for name in path:
		m = find_member(s, i, name)
		if m is None:
			raise KeyError("path element %r not found (path %r)" % (name, path))
		i = _skip_ws(s, m[1])
	return i


def insert_after(s, path, after_field, new_field, new_value):
	"""Insert `new_field: new_value` right after `after_field` in the object at
	`path`, matching that object's layout.  Returns the new text."""
	obj = find_object(s, path)
	m = find_member(s, obj, after_field)
	if m is None:
		raise KeyError("field %r not present in %r" % (after_field, path))
	if find_member(s, obj, new_field) is not None:
		raise KeyError("field %r already present in %r" % (new_field, path))
	_, _, value_end = m
	# Layout: does this object put its members on separate lines?
	obj_end = _scan_value(s, obj)
	body = s[obj:obj_end]
	encoded = json.dumps(new_field) + ": " + json.dumps(new_value)
	if "\n" in body:
		# expanded: reuse the indent of the member we insert after
		line_start = s.rfind("\n", 0, m[0]) + 1
		indent = s[line_start:m[0]]
		sep = ",\n" + indent
	else:
		sep = ", "
	return s[:value_end] + sep + encoded + s[value_end:]


def patch_file(path, edits):
	"""edits: list of (json_path, after_field, new_field, new_value).

	Applies them all, then re-parses and asserts every new value landed."""
	with open(path, "r", encoding="utf-8") as f:
		s = f.read()
	before = json.loads(s)
	# Apply from the END of the file backwards so earlier offsets stay valid.
	# find_object recomputes offsets each time, so order is not strictly
	# required -- but applying in reverse document order keeps each edit's
	# context untouched by its predecessors, which makes a failure easier to
	# localise.
	for jpath, after, field, value in edits:
		s = insert_after(s, jpath, after, field, value)
	after_doc = json.loads(s)
	# Verify: every edit landed, and NOTHING else moved.
	for jpath, _after, field, value in edits:
		node = after_doc
		for p in jpath:
			node = node[p]
		if node.get(field) != value:
			raise AssertionError("edit did not land: %r %s" % (jpath, field))
		node.pop(field)
	if after_doc != before:
		raise AssertionError("patch changed something other than the new fields in " + path)
	with open(path, "w", encoding="utf-8") as f:
		f.write(s)
	return len(edits)
