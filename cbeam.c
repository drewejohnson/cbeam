/* Copyright 2020 Andrew Johnson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *                               cbeam
 *
 * Reads the input file from standard in and outputs directly
 * to standard out. The following is (current) intended usage:
 *
 * $ cbeam < input.md > output.tex
 *
 * Task list in order of priority
 * TODO itemize and enumerate environments
 * TODO Better data structures for tracking environments
 * TODO Theme support
 * TODO Tables
 * TODO Inline captions e.g. ![This is the caption](image.pdf)
 * TODO Some command line processing
 * TODO In-line processing (urls, bold, italics)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // isspace

enum location {
  PREAMBLE,
  NO_FRAME,
  IN_FRAME,
};

enum environment {
  TEXT,
  FIGURE,
  NO_ENV,
};

enum location start_document(FILE* dest, int titlepage)
{
  fprintf(dest, "\n\\begin{document}\n");
  if (titlepage)
    fprintf(dest, "\n\\frame{\\titlepage}\n");
  return NO_FRAME;
}

enum location start_frame(FILE* dest) {
  fprintf(dest, "\n\\begin{frame}\n");
  return IN_FRAME;
}

enum location check_start_frame(FILE* dest, enum location loc) {
  switch (loc) {
    case PREAMBLE:
      start_document(dest, 1);
      return start_frame(dest);
    case NO_FRAME:
      return start_frame(dest);
    default:
      return IN_FRAME;
  }
}

enum special_token {
  NO,
  AUTHOR,
  DATE,
  CAPTION,
  LABEL,
  TOC,
  UNDEFINED,
};

enum special_token check_token(char* substr)
{
  if (substr[0] != ':')
    return NO;
  if (NULL != strstr(substr, ":author:"))
    return AUTHOR;
  if (NULL != strstr(substr, ":date:"))
    return DATE;
  if (NULL != strstr(substr, ":caption:"))
    return CAPTION;
  if (NULL != strstr(substr, ":label:"))
    return LABEL;
  if (NULL != strstr(substr, ":toc:"))
    return TOC;
  return NO;
}

int is_linebreak(char* line) {
  for (int i=0; i < strlen(line); ++i) {
    if (isspace(line[i]) == 0)
      return 0;
  }
  return 1;
}

char* strip_whitespace(char* str)
{
  // Remove leading whitespace by offsetting the
  // start of the string
  while (isspace((unsigned char)*str)) ++str;

  if (*str == 0)
    return str;

  char* end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) --end;
  end[1] = '\0';

  return str;
}

int process_special(enum special_token token, char* substr, FILE* dest)
{
  char* delim;
  char* lead;
  switch (token) {
    case AUTHOR:
      delim = ":author:";
      lead = "author";
      break;
    case DATE:
      if (NULL != strstr(substr, "today")) {
        fprintf(dest, "\\date{\\today}\n");
        return 0;
      }
      delim = ":date:";
      lead = "date";
      break;
    case LABEL:
      delim = ":label:";
      lead = "label";
      break;
    case CAPTION:
      delim = ":caption:";
      lead = "caption";
      break;
    case TOC:
      fprintf(dest, "\\frame{\\tableofcontents}\n");
      return 0;
    default:
      return 1;
  }
  substr = &substr[strlen(delim)];
  // Trim off new line character
  substr = strip_whitespace(substr);

  // TODO Trim whitespace
  fprintf(dest, "\\%s{%s}\n", lead, substr);
  return 0;
}

int get_heading_level(char* line)
{
  int level = 0;

  while (line[level] == '#') {
    ++level;
  }
  return level;
}

int process_heading(char* line, int level, FILE* dest)
{
  // Only support leading # for now
  char* heading = strip_whitespace(&(line[level]));

  if (level == 1) {
    fprintf(dest, "\\title{%s}\n", heading);
    return level;
  }

  fprintf(dest, "\n\\");
  for (int i = 0; i < level - 2; ++i)
    fprintf(dest, "sub");
  fprintf(dest, "section{%s}\n", heading);
  return level;
}

int process_image(char* line, FILE* dest)
{
  if (line[0] != '!' || line[1] != '[') {
    fprintf(stderr, "Malformed image line: %s\n", line);
    return 1;
  }

  int cap_start=2, cap_end=0, img_end=0;

  // Form: [caption](image)
  // Caption block can be empty

  for (int i=cap_start; i < strlen(line); ++i) {
      if (line[i] == ']') {
        if (cap_end != 0) {
          fprintf(stderr, "Malformed image line: %s\n", line);
          return 1;
        }
        cap_end = i;
      } else if (line[i] == ')') {
        if (img_end != 0) {
          fprintf(stderr, "Malformed image line: %s\n", line);
          return 1;
        }
        img_end = i;
      }
  }
  if (cap_end == 0 || img_end == 0 || img_end < cap_end) {
    fprintf(stderr, "Malformed image line: %s\n", line);
    return 1;
  }

  line[img_end] = '\0';

  fprintf(dest, "\\includegraphics[width=0.6\\textwidth,height=0.6\\textheight,keepaspectratio]{%s}\n",
      &(line[cap_end + 2]));

  if (cap_end > cap_start + 1) {
    fprintf(stderr, "In-line captions not supported at this moment\n");
  }

  return 0;
}

int process_title(char* line, FILE* dest)
{
  char* title = strip_whitespace(line);
  int end = strlen(title);

  if (title[end - 1] == '*' && title[end - 2] == '*') {
    title[end - 2] = '\0';
  }

  fprintf(dest, "\\frametitle{%s}\n", title);
  return 0;
}

int main(int argc, char* argv[])
{
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  enum special_token token;
  enum location current_loc = PREAMBLE;
  enum environment current_env = NO_ENV;
  int stat, list_level=0;
  FILE* destination = stdout;

  // Begin processing directly to stdout
  fprintf(destination, "\\documentclass{beamer}\n");
  fprintf(destination, "\\usepackage{graphicx}\n");

  while ((nread = getline(&line, &len, stdin)) != -1) {

    // Process special potentially non-frame information
    if ((token = check_token(line)) != NO) {
      if (token == TOC && current_loc == PREAMBLE) {
        start_document(destination, 1);
        current_loc = NO_FRAME;
      }
      process_special(token, line, destination);
      continue;

    } else if (line[0] == '#') {
      stat = get_heading_level(line);
      if (stat == 0) {
        fprintf(stderr, "Malformed heading: %s\n", line);
        return 1;
      } else if (stat > 1 && current_loc != NO_FRAME) {
        if (current_loc == PREAMBLE) {
          start_document(destination, 1);
        } else if (current_loc == IN_FRAME) {
          fprintf(destination, "\\end{frame}\n");
        }
        current_loc = NO_FRAME;
      }
      process_heading(line, stat, destination);
      continue;

    }

    if (is_linebreak(line)) {
      if (current_loc == IN_FRAME) {
        if (current_env != NO_ENV) {
          switch (current_env) {
            case FIGURE:
              fprintf(destination, "\\end{figure}\n");
              break;
            default:
              break;
          }
        current_env = NO_ENV;
        }
        fprintf(destination, "\\end{frame}\n");
        current_loc = NO_FRAME;
      }
      continue;
    }

    if (line[0] == '*' && line[1] == '*') {
      current_loc = check_start_frame(destination, current_loc);
      stat = process_title(&(line[2]), destination);
      if (stat != 0)
        return stat;
    } else if ((line[0] == '*' || line[0] == '-') && isspace(line[1])) {
      // Bullets
      fprintf(stderr, "Lists are not allowed for now: %s\n", line);
      return 1;
    } else if (isdigit(line[0]) && line[1] == '.' && isspace(line[2])) {
      // Numbered list
      fprintf(stderr, "Lists are not allowed for now: %s\n", line);
      return 1;
    } else if (line[0] == '!') {
      // Don't like having to put this everywhere...
      current_loc = check_start_frame(destination, current_loc);
      if (current_env != FIGURE) {
        fprintf(destination, "\\begin{figure}\n");
        current_env = FIGURE;
      }
      stat = process_image(line, destination);
      if (stat != 0) {
        return stat;
      }
    } else {
      // Plain text - throw directly to LaTeX
      current_loc = check_start_frame(destination, current_loc);
      fprintf(destination, line);
    }
  }
  free(line);

  if (current_loc == IN_FRAME)
    if (current_env == FIGURE) {
      fprintf(destination, "\\end{figure}\n");
    }
    fprintf(destination, "\\end{frame}\n");

  fprintf(destination, "\\end{document}\n");
  return 0;
}
