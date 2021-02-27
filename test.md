:author: Andrew Johnson
:date: today
:theme: Boadilla
:inner: rectangles
:pkg: hyperref

# This is the title

:toc:

**This is the frame title**
Frames can consist of plain text, lists (numbered or not), images.
You can have as long of a frame as you want, but no commands
are used to break up long slides.
Math font like $x=y$ is captured natively.

**Another frame**
Frames are broken up by empty lines. Contents of a paragraph
can run over multiple lines. If the frame title does not
appear directly above the frame, or at least attached to the block,
the title will not be applied to the desired frame.
Percentages should be escaped like \%% both to avoid expansion from the
C-parser and then to avoid creating a comment in TeX.

**Empty frame**

This frame has no title. 
Sad frame.

## New section

:sectionpage:

**Lists**
- Only single level lists are supported for now
* You can mix symbols and they will be treated as the same bulletted list
1. Switching to numbers will terminate the list and switch to enumerations
1. Just like markdown, number doesn't matter, just that it's a numeric entry followed by a period and a space
8. This is really the third item. The next line is an edge case
1111111111. a

**Long lines**
- The contents of a single line are passed to a list item
- Line continuation is not supported as each line is treated separately from previous and next lines
- This hopefully discourages very long list items

### Subsection

**Images too!**
![](assets/is_it_worth_the_time.png)
:caption: XKCD 1205 (CC-BY NC 2.5)
:label: figure0
Original link: \url{https://xkcd.com/1205/}.
License: \url{https://creativecommons.org/licenses/by-nc/2.5/}

## Conclusion

**Conclusion**
This is a pretty neat tool (I think so).
It has some limits, and there is more to add.
