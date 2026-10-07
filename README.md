# Lexical Analyzer

A compiler front-end demo that splits source text into tokens. A C++ server classifies the input, and a browser page shows each lexeme with its category.

## What it recognizes

| Category | Examples |
| --- | --- |
| `KEYWORD` | `if`, `else`, `for`, `while`, `return`, `int`, `float` |
| `IDENTIFIER` | `count`, `total_sum`, `_value` |
| `NUMERIC_CONSTANT` | `42`, `3.14`, `1.5e10`, `2E^3` |
| `OPERATOR` | `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<=`, `>=`, `<<`, `>>`, `<>` |
| `SPECIAL_CHAR` | `( ) { } [ ] ; , . :` |
| `UNKNOWN` | Any other character |

Whitespace is skipped and is not returned as a token.

## Project layout

| Path | Role |
| --- | --- |
| `main.cpp` | Lexer and HTTP server |
| `httplib.h` | Header-only HTTP library ([cpp-httplib](https://github.com/yhirose/cpp-httplib)) |
| `index.html` | Analyzer page |
| `app.js` | Sends source text to the server and renders the token table |
| `presentation/` | Slide deck for the project |
| `lexer_server2.exe` | Prebuilt Windows server |

## Run it

The page talks to `http://localhost:8080`. Start the server first, then open the page.

**Windows, using the included binary**

```text
lexer_server2.exe
```

**Build from source**

Windows (MinGW):

```text
g++ -std=c++17 main.cpp -o lexer_server.exe -lws2_32
lexer_server.exe
```

Linux or macOS:

```text
g++ -std=c++17 main.cpp -o lexer_server -pthread
./lexer_server
```

When the server prints `Lexer server running at http://localhost:8080`, open `index.html` in a browser, type or paste code, and click **Analyze**.

## API

`POST /analyze`

```json
{ "code": "x = 5 + 10;" }
```

```json
[
  { "lexeme": "x", "type": "IDENTIFIER" },
  { "lexeme": "=", "type": "OPERATOR" },
  { "lexeme": "5", "type": "NUMERIC_CONSTANT" },
  { "lexeme": "+", "type": "OPERATOR" },
  { "lexeme": "10", "type": "NUMERIC_CONSTANT" },
  { "lexeme": ";", "type": "SPECIAL_CHAR" }
]
```

The server accepts cross-origin requests from the local page (`Access-Control-Allow-Origin: *`).

## Presentation

Open `presentation/index.html` for the slide deck. Use the on-screen buttons or the arrow keys to move between slides. Print from the browser if you need a PDF.
