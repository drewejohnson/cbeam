# cbeam

A Markdown(ish) to beamer converter.

I really like making beamer presentations, but I find they have a lot
of extra stuff to write
```latex
\begin{figure}
  \begin{itemize}
    \item{First item}
    \begin{itemize}
      \item{Sub point}
    \end{itemize}
  \end{itemize}
\end{frame}
\begin{frame}
  \begin{figure}
    \includegraphics{image}
    \caption{Caption}
    \label{ref}
  \end{figure}
\end{frame}
```

Enter ``cbeam``. It is a simple-but-not-perfect converted between Markdown files
and beamer tex files. See the limitations section for what it can't do (yet or ever).

## Installation

A super boring Makefile is provided, allowing you to make
```
$ make
```
and test
```
$ make test
``` 
the utility. Your c compiler of choice should work fine.
```
$ gcc cbeam.c -o cbeam
```

## Usage

``cbeam`` reads from the standard input and writes to standard output, e.g.
```
$ ./cbeam < input.md > output.tex
```
will convert the contents of ``input.md`` to ``output.tex``, which __should__
be a valid beamer file. For a quick read, the file ``test.md`` demonstrates
most of the capabilities and rules.

### Rules


Frames are considered to be lines of text separated by one or more empty
lines. Lines are processed separate from previous and subsequent lines.
It is the opinion of the author that frames should not have too much text.
Lists especially should be fairly short.

A frame title can be configured by starting a line with two asterisks,
```

**Frame title**
Frame contents

**New frame**
Some more stuff
```

The presentation title and (sub)sections are configured by headings,
```
# Title of the presentation

## Section title

### Subsection
#### Subsubsection

## New section
```
Spacing does not matter between headings.

The parser assumes you are configuring the preamble (author, title, date, etc.)
A non-empty line that does not contain a preamble command (see below) or the
heading level 1 line will start parsing the main document. To keep things
on the safe side, put your ``# Title`` towards the top of your document,
before you start writing frames of other non-preamble commands.

Lines are processed as they are read. Any malformed lines or
content out of place may terminate the program, and/or produce
a non-valid tex file. One exception is the ``:toc:`` command, which
can be placed in the preamble, but will immediately switch to parsing the
main document. This means that the following
```
:author: Me, myself, and I
:toc:
:date: The future
```
will place the ``\date`` tex directive inside your main work and potentially
not render well.

The first slide is always a title slide.

Bulleted and numbered lists are captured directly from Markdown.
Bullets create ``itemize`` environments and must start with either
``* `` or ``- ``. Note the space. Numbered lists don't have to be ordered,
just matching the pattern ``^[0-9]+\. ``. The content following any initial 
whitespace is placed directly inside an ``\item{content}`` directive.

Some text command are provided to help configure the
preamble and table of contents. Many of the commands are of the form
``:cmd: Value`` where the ``Value`` argument will be dumped into
the corresponding tex command.

### Preamble commands

- ``:author: Author list`` configures the author list. 
- ``:date: today`` configures the date. If ``today`` is found, the special ``\today``
  value will be set
- ``:theme: Theme`` configures the primary document theme
- ``:colors: scheme`` configures the color theme
- ``:inner: inner`` configures the inner theme
- ``:outer: outer`` configures the outer theme

### Main document commands
These commands should be included inside the main body of your document.

- ``:toc:`` triggers a frame devoted to the table of contents. No arguments are used
- ``:caption: Caption`` sets a (figure only) caption
- ``:label: ref`` sets a reference (for a figure. Sections etc. maybe?)

## Limitations

### Not yet
In order of decreasing priority

- Nested lists
- In-line captions for figures
- Tables (can use ``\input{table.tex}`` as a work around)
- Short title directives like ``\author[M.P]{The honorable Monty Python}``
- Section overviews
- In-line bold and italics using ``**markdown** __commands__`` (``\textbf{markdown} \textit{should}`` work)
- Some input processing (read from file, write to file)
- Package control (autoloads just ``graphicx``)

### Maybe ever

- Two columns
- ``tikz`` without using ``\input{myfigure.tex}``
- Transitions

## License

Copyright 2020 Andrew Johnson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Other licenses

The glorious XKCD comic that serves as an example image can be found
at https://xkcd.com/1205/, copyright Randall Monroe under
creative commons attribution non-commercial 2.5 license.
https://creativecommons.org/licenses/by-nc/2.5/
