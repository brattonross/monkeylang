package repl

import (
	"bufio"
	"fmt"
	"io"

	"github.com/brattonross/monkeylang/evaluator"
	"github.com/brattonross/monkeylang/lexer"
	"github.com/brattonross/monkeylang/object"
	"github.com/brattonross/monkeylang/parser"
)

const PROMPT = ">> "

func Start(r io.Reader, w io.Writer) {
	scanner := bufio.NewScanner(r)
	env := object.NewEnvironment()

	for {
		fmt.Fprint(w, PROMPT)
		scanned := scanner.Scan()
		if !scanned {
			return
		}

		line := scanner.Text()
		l := lexer.New(line)
		p := parser.New(l)

		program := p.ParseProgram()
		if len(p.Errors()) > 0 {
			printParserErrors(w, p.Errors())
			continue
		}

		evaluated := evaluator.Eval(program, env)
		if evaluated != nil {
			io.WriteString(w, evaluated.Inspect())
			io.WriteString(w, "\n")
		}
	}
}

func printParserErrors(w io.Writer, errors []string) {
	fmt.Fprintf(w, "Woops! We ran into some monkey business here!\n")
	fmt.Fprintf(w, " parser errors:\n")
	for _, msg := range errors {
		fmt.Fprintf(w, "\t%s\n", msg)
	}
}
