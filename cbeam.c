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
 * TODO Better data structures for tracking environments
 * TODO Tables
 * TODO Inline captions e.g. ![This is the caption](image.pdf)
 * TODO Some command line processing
 * TODO In-line processing (urls, bold, italics)
 */

#include <ctype.h> // isspace
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum location {
  PREAMBLE,
  NO_FRAME,
  IN_FRAME,
};

enum environment {
  TEXT,
  FIGURE,
  LIST_BULLET,
  LIST_NUMBER,
  NO_ENV,
};

/**
 * Start the document section, optionally with a title slide
 * \param dest Writable destination file
 * \param titlepage integer flag used to write title slide or not
 * \return current no-frame environment
 */
enum location start_document(FILE *dest, int titlepage)
{
  fprintf(dest, "\n\\begin{document}\n");
  if (titlepage)
    fprintf(dest, "\n\\frame{\\titlepage}\n");
  return NO_FRAME;
}

//! Write a frame start and return in-frame environment
enum location start_frame(FILE *dest)
{
  fprintf(dest, "\n\\begin{frame}\n");
  return IN_FRAME;
}

/**
 * Check if a frame should be started, starting if necessary
 *
 * \param dest Writable destination file
 * \param loc Current location in the document (preamble, in or out of frame)
 * \return In-frame enumeration
 */
enum location check_start_frame(FILE *dest, enum location loc)
{
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

/**
 * End a specific environment
 * \param dest Writable destination file
 * \param env Current environment
 * \return 0 if the environment was safely ended, non-zero otherwise
 */
int end_environment(FILE *dest, enum environment env)
{
  switch (env) {
  case FIGURE:
    fprintf(dest, "\\end{figure}\n");
    break;
  case LIST_BULLET:
    fprintf(dest, "\\end{itemize}\n");
    break;
  case LIST_NUMBER:
    fprintf(dest, "\\end{enumerate}\n");
    break;
  default:
    break;
  }
  return 0;
}

enum special_token {
  NO,
  AUTHOR,
  DATE,
  CAPTION,
  LABEL,
  TOC,
  SECTION_PAGE,
  UNDEFINED,
  THEME,
  PACKAGE,
  COLOR_THEME,
  INNER_THEME,
  OUTER_THEME
};

/**
 * Parse a substring for a special command
 * \param substr String potentially containing a special command
 * \return Indicator of the discovered substring
 */
enum special_token check_token(char *substr)
{
  if (substr[0] != ':')
    return NO;
  if (NULL != strstr(substr, ":sectionpage:"))
    return SECTION_PAGE;
  if (NULL != strstr(substr, ":author:"))
    return AUTHOR;
  if (NULL != strstr(substr, ":date:"))
    return DATE;
  if (NULL != strstr(substr, ":pkg:"))
    return PACKAGE;
  if (NULL != strstr(substr, ":caption:"))
    return CAPTION;
  if (NULL != strstr(substr, ":label:"))
    return LABEL;
  if (NULL != strstr(substr, ":toc:"))
    return TOC;
  if (NULL != strstr(substr, ":theme:"))
    return THEME;
  if (NULL != strstr(substr, ":colors:"))
    return COLOR_THEME;
  if (NULL != strstr(substr, ":inner:"))
    return INNER_THEME;
  if (NULL != strstr(substr, ":outer:"))
    return OUTER_THEME;
  return NO;
}

/**
 * Check if a line is empty
 * \param line Single line of text
 * \return Flag denoting if the entire line is full of whitespace
 */
int is_linebreak(char *line)
{
  for (int i = 0; i < strlen(line); ++i) {
    if (isspace(line[i]) == 0)
      return 0;
  }
  return 1;
}

//! Removing leading and trailing whitespace from a line
char *strip_whitespace(char *str)
{
  // Remove leading whitespace by offsetting the
  // start of the string
  while (isspace((unsigned char)*str))
    ++str;

  if (*str == 0)
    return str;

  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    --end;
  end[1] = '\0';

  return str;
}

/**
 * Write content depending on a parsed special token
 * \param special_token User command found in the substring
 * \param substr Line provided by user containing the special command
 * \param dest Writable destination file
 * \return Status if the token was processed and content written
 */
int process_special(enum special_token token, char *substr, FILE *dest)
{
  char *delim;
  char *lead;
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
  case SECTION_PAGE:
    fprintf(dest, "\\frame{\\sectionpage}\n");
    return 0;
  case THEME:
    delim = ":theme:";
    lead = "usetheme";
    break;
  case PACKAGE:
    delim = ":pkg:";
    lead = "usepackage";
    break;
  case COLOR_THEME:
    delim = ":colors:";
    lead = "usecolortheme";
    break;
  case INNER_THEME:
    delim = ":inner:";
    lead = "useinnertheme";
    break;
  case OUTER_THEME:
    delim = ":outer:";
    lead = "useoutertheme";
    break;
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

//! Return the number of leading heading markers
int get_heading_level(char *line)
{
  int level = 0;

  while (line[level] == '#') {
    ++level;
  }
  return level;
}

/**
 * Process the heading, making title or (sub)sections
 * \param line User-provided line including initial heading marks
 * \param level Heading level (1 for title, 2 for section, 3+ for subsections
 * \param dest Writable destination file
 * \return level is passed back
 */
int process_heading(char *line, int level, FILE *dest)
{
  // Only support leading # for now
  char *heading = strip_whitespace(&(line[level]));

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

/**
 * Process and write an image directive
 * \param line Markdown image command ![](image)
 * \param dest Writable destination file
 * \return 0 on sucess, non-zero if the line is malformed
 */
int process_image(char *line, FILE *dest)
{
  if (line[0] != '!' || line[1] != '[') {
    fprintf(stderr, "Malformed image line: %s\n", line);
    return 1;
  }

  int cap_start = 2, cap_end = 0, img_end = 0;

  // Form: [caption](image)
  // Caption block can be empty

  for (int i = cap_start; i < strlen(line); ++i) {
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

  fprintf(dest,
          "\\includegraphics[width=0.8\\textwidth,height=0.6\\textheight,"
          "keepaspectratio]{%s}\n",
          &(line[cap_end + 2]));

  if (cap_end > cap_start + 1) {
    fprintf(stderr, "In-line captions not supported at this moment\n");
  }

  return 0;
}

/**
 * Create a frame title from a character array
 *
 * Removes leading and trailing whitespace from the
 * line, and trims off the tail "**" if they exist
 *
 * \param line Character array to be processed
 * \param dest Writable destination file
 * \return 0 on success, 1 one failure
 */
int process_title(char *line, FILE *dest)
{
  char *title = strip_whitespace(line);
  int end = strlen(title);

  if (title[end - 1] == '*' && title[end - 2] == '*') {
    title[end - 2] = '\0';
  }

  fprintf(dest, "\\frametitle{%s}\n", title);
  return 0;
}

/**
 * Create a itemize item from a line
 *
 * Expects the line to match the pattern "^[*|-] (.*)" where the
 * later group will be placed inside an item command. Writes to
 * stderr on failure.
 *
 * \param line Character array to be processed
 * \param dest Writable destination file
 * \return 0 on success, 1 on failure
 */
int process_bullets(char *line, FILE *dest)
{
  if ((line[0] != '*' || line[0] != '-') && !isspace(line[1])) {
    fprintf(stderr, "Malformed bulleted list: Must match '[*|-] ': %s\n", line);
    return 1;
  }
  fprintf(dest, "\\item{%s}\n", strip_whitespace(&(line[2])));
  return 0;
}

/**
 * Determine the starting position of an enumerated list
 *
 * A little complicated, because it does support numbers
 * like "12. " even though they don't get converted directly.
 * check_enumerate("1. Hello world") would return 0, while
 * check_enumerate("111. Hello world") would return 2. A return
 * of -1 indicates the line is not well formed.
 *
 * \param line Character array that may or may contain an enumeration
 * \return Index of the last numeric character before the start of the content.
 */
int check_enumerate(char *line)
{
  int location = 0;
  for (location; location < strlen(line); ++location) {
    if (!isdigit(line[location])) {
      break;
    }
  }
  if (location == 0 || location >= strlen(line) - 3) {
    return -1;
  }
  if (line[location] != '.' || !isspace(line[location + 1])) {
    return -1;
  }
  return location - 1;
}

/**
 * Create a new enumerate item
 *
 * Expects a pattern of ^[0-9]*\.
 * \param line Character array to be parsed
 * \param dest Writable destination file
 * \return 0 on success, 1 if the line is incorrectly formed
 */
int process_enumerate(char *line, FILE *dest)
{
  if (!isdigit(line[0]) && line[1] != '.' && !isspace(line[3])) {
    fprintf(stderr, "Malformed numbered list. Must match '[0-9]*\\. '", line);
    return 1;
  }
  fprintf(dest, "\\item{%s}\n", strip_whitespace(&(line[3])));
  return 0;
}

int main(int argc, char *argv[])
{
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  enum special_token token;
  enum location current_loc = PREAMBLE;
  enum environment current_env = NO_ENV;
  int stat, list_level = 0;
  FILE *destination = stdout;

  // Begin processing directly to stdout
  fprintf(destination, "\\documentclass{beamer}\n");
  fprintf(destination, "\\usepackage{graphicx}\n");

  while ((nread = getline(&line, &len, stdin)) != -1) {

    // Process special potentially non-frame information
    if ((token = check_token(line)) != NO) {
      if ((token == TOC || token == SECTION_PAGE) && current_loc == PREAMBLE) {
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
          end_environment(destination, current_env);
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
      if (current_env != LIST_BULLET) {
        end_environment(destination, current_env);
        fprintf(destination, "\\begin{itemize}\n");
        current_env = LIST_BULLET;
      }
      if ((stat = process_bullets(line, destination)))
        return stat;
    } else if (isdigit(line[0])) {
      stat = check_enumerate(line);
      if (stat >= 0) {
        if (current_env != LIST_NUMBER) {
          end_environment(destination, current_env);
          fprintf(destination, "\\begin{enumerate}\n");
          current_env = LIST_NUMBER;
        }
        if (stat = process_enumerate(&(line[stat]), destination))
          return stat;
      }
    } else if (line[0] == '!') {
      current_loc = check_start_frame(destination, current_loc);
      if (current_env != FIGURE) {
        end_environment(destination, current_env);
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

  if (current_loc == IN_FRAME) {
    end_environment(destination, current_env);
    fprintf(destination, "\\end{frame}\n");
  }

  fprintf(destination, "\\end{document}\n");
  return 0;
}
