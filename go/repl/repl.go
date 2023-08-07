package repl

import (
	"bufio"
	"fmt"
	"io"

	"github.com/brattonross/interpreter/lexer"
	"github.com/brattonross/interpreter/parser"
)

const PROMPT = ">> "

func Start(r io.Reader, w io.Writer) {
	scanner := bufio.NewScanner(r)

	for {
		fmt.Fprintf(w, PROMPT)
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

		io.WriteString(w, program.String())
		io.WriteString(w, "\n")
	}
}

func printParserErrors(w io.Writer, errors []string) {
	fmt.Fprintf(w, "Woops! We ran into some monkey business here!\n")
	fmt.Fprintf(w, " parser errors:\n")
	for _, msg := range errors {
		fmt.Fprintf(w, "\t%s\n", msg)
	}
}
