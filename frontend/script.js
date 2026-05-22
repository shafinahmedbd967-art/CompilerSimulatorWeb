//korbo lorbo jitbo
//7tab plus theory white
document.addEventListener('DOMContentLoaded', () => {
  const tabs = document.querySelectorAll('.tab-btn');
  const sections = document.querySelectorAll('.tab');

  // Tab switching
  tabs.forEach(btn => {
    btn.addEventListener('click', () => {
      tabs.forEach(b => b.classList.remove('active'));
      sections.forEach(s => s.classList.remove('active'));
      btn.classList.add('active');
      document.getElementById(btn.dataset.tab).classList.add('active');
    });
  });

  const exprInput = document.getElementById('exprInput');
  const startBtn = document.getElementById('startBtn');
  const examples = document.getElementById('examples');

  const phaseIds = ['lexical','syntax','semantic','intermediate','optimization','assembly'];

  examples.addEventListener('change', () => exprInput.value = examples.value);

  // Generate theory text
  function getTheory(phase, expr) {
    switch (phase) {
      case 'LEXICAL': return `Lexical Analysis breaks the input expression "${expr}" into tokens and builds the symbol table.`;
      case 'SYNTAX': return `Syntax Analysis parses the token stream of "${expr}" to create a parse tree, checking grammar rules.`;
      case 'SEMANTIC': return `Semantic Analysis validates the meaning of "${expr}", data types, and operations.`;
      case 'INTERMEDIATE': return `Intermediate Code Generation converts "${expr}" into three-address code (TAC) for optimization.`;
      case 'OPTIMIZATION': return `Code Optimization improves the TAC of "${expr}" by folding constants and removing redundancies.`;
      case 'ASSEMBLY': return `Assembly Code Generation translates optimized TAC of "${expr}" into CPU instructions.`;
      default: return '';
    }
  }

  // Create phase content card
  function createPhaseContent(content, theoryText) {
    return `
      <div class="phase-card">
        <div class="phase-header">Output</div>
        <div class="phase-content">${content}</div>
        <div class="phase-content theory">${theoryText}</div>
      </div>`;
  }

  // Format all phases
  function formatPhases(expr, data) {
    if(data.error) {
      const phaseDiv = document.getElementById('tab-lexical');
      phaseDiv.innerHTML = createPhaseContent(`ERROR: ${data.error.message}\nPartial Lexical Output:\n${data.lexical.tokens}`, getTheory('LEXICAL', expr));
      return;
    }

    const outputs = {
      'LEXICAL': `Expression: ${expr}\nTokens:\n${data.lexical.tokens}\nSymbol Table:\n${data.lexical.symtab}\nNormalized:\n${data.lexical.normalized}`,
      'SYNTAX': `Parse Tree:\n${data.syntax.parse_tree}\nVariable Mapping:\n${data.syntax.varmap}`,
      'SEMANTIC': `Semantic Tree:\n${data.semantic.sem_tree}\nVariable Types:\n${data.semantic.var_types}`,
      'INTERMEDIATE': `TAC:\n${data.intermediate.tac}`,
      'OPTIMIZATION': `Optimized TAC:\n${data.optimization.opt_tac}\nInfo:\n${data.optimization.info}`,
      'ASSEMBLY': `ASM:\n${data.assembly.asm}`
    };

    phaseIds.forEach((phase, index) => {
      const id = `tab-${phase}`;
      const theoryText = getTheory(phase.toUpperCase(), expr);
      document.getElementById(id).innerHTML = createPhaseContent(outputs[phase.toUpperCase()], theoryText);
    });
  }

  startBtn.addEventListener('click', () => {
    const expr = exprInput.value.trim();
    if(!expr) return alert('Please enter an expression.');

    fetch('/compile', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({expression: expr})
    })
    .then(res => res.json())
    .then(data => formatPhases(expr,data))
    .catch(err => {console.error(err); alert('Compilation failed. Check console.')});
  });
});
