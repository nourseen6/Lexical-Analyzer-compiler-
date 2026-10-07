const analyzeBtn = document.getElementById("analyzeBtn");
const codeInput = document.getElementById("codeInput");
const resultsBody = document.getElementById("resultsBody");
const statusLabel = document.getElementById("status");

function renderTokens(tokens) {
  if (!Array.isArray(tokens) || tokens.length === 0) {
    resultsBody.innerHTML = "<tr><td colspan='2'>No tokens found.</td></tr>";
    return;
  }

  resultsBody.innerHTML = tokens
    .map((token) => {
      const lexeme = (token.lexeme ?? "").replace(/</g, "&lt;").replace(/>/g, "&gt;");
      const type = token.type ?? "UNKNOWN";
      return `<tr>
        <td>${lexeme}</td>
        <td class="${type}">${type}</td>
      </tr>`;
    })
    .join("");
}

async function analyzeCode() {
  const code = codeInput.value;
  statusLabel.textContent = "Analyzing...";

  try {
    const response = await fetch("http://localhost:8080/analyze", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ code }),
    });

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const tokens = await response.json();
    renderTokens(tokens);
    statusLabel.textContent = `Analyzed ${tokens.length} token(s).`;
  } catch (error) {
    resultsBody.innerHTML =
      "<tr><td colspan='2'>Failed to reach backend. Start C++ server on port 8080.</td></tr>";
    statusLabel.textContent = `Error: ${error.message}`;
  }
}

analyzeBtn.addEventListener("click", analyzeCode);

codeInput.value = "x = 5 + 10;";
