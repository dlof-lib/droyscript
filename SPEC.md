# droy script x — Language Specification v0.1

## Purpose
`droy script x` is a small **data/graph description language**. It exists to
declare data as **groups of rows**, derive **collections** from them with
`equals` filters, connect records into **bridges** (graph edges between
`id`s), and build **maps** (key → value tables) — then act on all of it with
a tiny set of built-in functions. It is designed to be trivial to lex/parse
so it can run instantly inside a mobile editor.

## File envelope
Every program starts and ends with the exact markers:

```
@see.droy=start
...body...
@see.droy=end
```

Anything outside the markers is ignored (so files can carry comments/headers
above `start` and notes below `end`).

## Sigils
| Sigil | Role                                             |
|-------|---------------------------------------------------|
| `@`   | opens a **group-level** construct: `@group`, `@collection`, `@bridge`, `@map`, `@see`, `@end` |
| `$`   | opens a **row** (a record) inside a group          |
| `.`   | calls a **method** on a named entity: `Users.equals(...)` |
| `~`   | calls a **built-in** (system) function: `~print(...)` |
| `=>`  | **forward** a result — pipe/derive-into, or key→value in a map, or edge source→target |
| `<=`  | **backward** assign — bind a name to the result of an expression |
| `=`   | **equals** — set a field, or the filter test in `.equals(field=value)` |
| `""`  | string literal; an empty `""` denotes "no value" (void) |

## Concepts / keywords
`group row collection equals number bridge id name edge map`

- **group** — a named table of rows. `@group Users ... @end`
- **row** — one record inside a group: `$row id=1 name="Ali" number=100`
- **collection** — a named, derived set of rows: built from a group with a
  `.equals(field=value)` filter, or built from an explicit literal list.
- **bridge** — a named graph: a set of `edge`s connecting two `id`s.
  `edge id=1 => id=2` (source `=>` target).
- **map** — a named dictionary of `"key" => value` pairs.
- **number / name / id / equals / edge** — field/role names used inside rows,
  edges and filters; not reserved outside those positions.

## Grammar (informal EBNF)

```
program     := "@see.droy=start" stmt* "@see.droy=end"
stmt        := groupDecl | collectionDecl | bridgeDecl | mapDecl | builtinCall
groupDecl      := "@group" IDENT rowDecl* "@end"
rowDecl        := "$row" field*
collectionDecl := "@collection" IDENT ("<=" derive | rowDecl*) "@end"?
derive         := IDENT "." "equals" "(" field ")"
bridgeDecl     := "@bridge" IDENT edgeDecl* "@end"
edgeDecl       := "edge" field "=>" field
mapDecl        := "@map" IDENT mapEntry* "@end"
mapEntry       := STRING "=>" value
field          := IDENT "=" value
value          := STRING | NUMBER | IDENT
builtinCall    := "~" IDENT "(" arg? ("," arg)* ")"
arg            := IDENT | STRING | NUMBER
```

## Built-ins
- `~print(X)` — print a group/collection/bridge/map/row to stdout.
- `~len(X)` — number of rows/edges/entries in X.
- `~sum(Group, field)` — numeric sum of a field across a group's rows.

## Worked example

```droy
@see.droy=start

@group Users
    $row id=1 name="Ali"  number=100
    $row id=2 name="Omar" number=200
    $row id=3 name="Sara" number=100
@end

@collection ActiveUsers <= Users.equals(number=100)
@end

@bridge Friendship
    edge id=1 => id=2
    edge id=2 => id=3
@end

@map Settings
    "theme"   => "dark"
    "version" => 1
@end

~print(Users)
~print(ActiveUsers)
~print(Friendship)
~print(Settings)
~len(Users)
~sum(Users, number)

@see.droy=end
```

This is the reference program used by the C++ engine's test suite and by the
Android app's built-in sample.
