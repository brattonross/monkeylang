package repl

import (
	"bufio"
	"fmt"
	"io"

	"github.com/brattonross/interpreter/lexer"
	"github.com/brattonross/interpreter/token"
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

		for tok := l.NextToken(); tok.Type != token.EOF; tok = l.NextToken() {
			fmt.Fprintf(w, "%+v\n", tok)
		}
	}
}
