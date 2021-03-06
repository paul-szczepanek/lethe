`S.K.I.M.`
==========
Selectable Keyword Interaction Markup
-------------------------------------

The language drives the text adventure by serving text interspersed with
keywords that are highlighted. The user clicks on the keyword to gain access
to actions possible on that keyword which when executed serve further text.

Internal logic relies on nouns which are represented by keywords in the text and
verbs belonging to that noun that are represented by the possible actions. Nouns
may contain text values and an integer value. Verbs that define available
actions may contain instructions and conditions that provide logic for the
action.

The text enclosed in double quotes `" "` gets printed on the screen.

Whitespace is ignored and may be used for indentation to aid readability.

> Comments in the manual, indented like this one, may mention things that are
> explained fully in subsequent sections.

---

This is the reference document, not a tutorial. For a primer in the language
read the tutorial story that explains all the basic functionality.

The language is fairly compact but still offers enough functionality to warrant
a lengthy manual but keep in mind that this is showing all the functionality.
When writing a story you will probably use a subset of it. Some features
are useful only in particular story styles or within the menu system. 

---

### `Noun Definition _________________________________________ [] [[]]`

`[noun]` - begin definition of the noun. This delimits noun definitions and
           needs to start on a new line.

Nouns can't start with numbers (`1noun` would be an illegal name) but may
contain numbers and spaces. Leading and trailing spaces will be ignored. Use
alphanumeric characters. Nouns need to have unique names.

> Nouns are Case **Sensitive**!
> Keep that in mind if you start a sentence with a keyword and use a capital
> letter and the noun uses a lower case letter (use `<Keyword[noun]>` then).

`[noun[pattern]]` - defines a noun using a pattern.

Patterns get prepended to the definition and every occurrence of the pattern
name gets replaced with the noun name of the noun being defined.

> multiple patterns can be used using commas `[noun[pattern1, pattern2]]`

`[[pattern]]` - this defines a pattern for nouns to use.

They are not evaluated and need to be defined before they are used.
They are essentially macros that get prepended to a noun definition.

### `Values _________________________________________________ [ = ]`

`[noun[pattern] = value1, value2, #9]` - sets the initial values of the noun.

Prepend integers with `#`. If no `#` is used then the number is treated as
text.

There can only ever be one integer value. `[noun = #1, #7, #8]` will set the
integer value to 8.

All nouns have an integer value 0 until it's changed. The integer value
doesn't count as a regular text value.

> In the initial assignment, you can't force noun evaluations with `@`.

### `Verb Definition _____________________________________________ [:]`

Verbs define the actions possible in the keyword menu.

- `[:verb]` - past this is where execution goes when you choose the action.
- `[:verb1 :verb2]` - only the first one is visible in the drop down menu
                      but both work (internal alias).
- `[: :hidden verb]` - leaving the name of the first verb blank will make
		       the action hidden from the list (space between `: :` is 
		       optional, like all other whitespace).

You can't have multiple verb definitions using the same name. Each
previous verb definition using a name that is being redefined will be
removed, even if it also had other names (blank names don't count).

### `Conditions _________________________________________________ ?`

Value comparisons:

- `?value1 += value2` - does value1 contain value2.
- `?value1 -= value2` - does value1 not contain value2.
- `?value1 = value2` - does value1 contain value2 and nothing else.

If you omit the left side of the condition, it is implied that you mean the
values that the noun currently being defined contains.

`?= value1` - does the noun being defined contain value1.

> An implied noun will expand to `@noun` and/or `#noun` based on the contents
> of the expression on the right.

These will try to compare the integer value of a:

`?#a = #4` `?#a <= #5` `?#a >= #2` `?#a < #4` `?#a > #8`

If no comparison is present it will return true if it has any contents.

`?value` - is true if integer greater than 0 or has text values.

Conditions may be placed above verb definitions. Apart from being a top
level condition they will hide the verb from the drop down menu if the
condition is not met.

### `Instructions ______________________________________________ !`

`!b:verb` - execute `[:verb]` in noun `[b]` (then return and continue).

Assignment instructions:

- `!noun += value` - add value to noun.
- `!noun -= value` - remove value from noun.
- `!noun = value` - set values of noun to value (removes previous values).
- `!= value` - missing noun defaults to the one being defined.
- `!noun=` - delete all values of noun (except the integer value).
- `!noun = #0` - set the integer value of noun to 0.

What is on the left side is the name of the noun you want to assign to.

> Using @ might not work as you expect as @noun will evaluate to its values.
> `!@noun = value` will not assign value to noun. It will assign value to
> nouns with names that match existing values of noun.

Instructions need to be below a verb definition.

### `Evaluation ______________________________________________ @ #`

Rather than using value literals (numbers and strings) for an instruction or
condition you can get the values contained in a noun. `#noun` evaluates to the
integer value and `@noun` to string values of the noun.

- `!noun = string` - assign the value string to noun.
- `!noun1 = @noun2` - assign the text values of noun2 to noun1
- `!noun1 = #noun2` - assign the integer value of noun1 the integer value
                        of noun2.

> If letters follow `#` than rather than a integer literal it's assumed that
> what follows is a noun name (nouns can't start with digits).

Numbers are handled wholly separate so if you wanted to copy both strings
and the integer value from noun2 to noun1 you would need to explicitly say:

`!noun1 = @noun2 + #noun2`

You can  use evaluations in instructions and conditions.

`!@noun1:@noun2` - this will go to noun1_value:noun2_value. If there are
                     multiple values it will execute them all in sequence.

You can nest @ evaluations.

`!@@noun1:verb` - will go to noun3:verb, if noun1 has a value called noun2
                    and there is a noun2 that has a value called noun3.

### `Arithmetic __________________________________________ ( ) - + * /`

You can do arithmetic operations on numbers with `+ - / * ( )`.

You can also add and subtract text values. Multiplying text values
concatenates them. Dividing text values returns values present in both
values.

Let's assume we have two nouns defined like so:

- `[noun1 = A, C]`
- `[noun2 = D, C]`

Then the following statements are all true:

- `?@noun1 = A + C`
- `?@noun1 * @noun2 = AD + AC + CD + CC`
- `?@noun1 / @noun2 = C`

Order of operations is left to right regardless of type so if you need
a different order of evaluation you can use parenthesis `( )`.

### `Code Blocks _________________________________________ { } & | !<<`

`{ }` are used for blocks. If no `{ }` are used then the code block is assumed
to end at the next condition. Implied block of a top level condition
(condition above a verb) ends with the next top level condition.

All the braces `{ }` below can be omitted without changing the meaning.

```
[noun]
?top condition1 {
  [:verb1]
    ?condition1 {
      "text"
    }
    ?condition2 {
      "text"
    }
}
?top condition2 {
  [:verb2]
    "text"
}
```

You can use `&` and `|` to create series of conditions:

- `?condition1 & condition2` - will short circuit, first negative result stops.
- `?condition1 | condition2` - will short circuit, first positive result stops.

Execution will continue to the end of the brace and return to the parent block
unless a break instruction `!<<` is encountered.

- `!<<` - breaks out of the current and parent block.
- `!<<<` - breaks up three levels up.
- ...and so on, add more `<` for more levels.

You cannot put a '!<<' in a series. Needs to be a standalone instruction.

> Note that a single `<` as in `!<` is illegal (and would be pointless).

### `Else Clause ______________________________ \n ?statement { } { }`

If you use `{ }` to define a scope of a condition you can define the else
clause (the set of instruction that get executed if the condition fails) by
placing it within a set of '{ }' immediately following the previous pair.

```
?condition {
  statement when condition true
} {
  statement when condition false
}
```

> If you want to chain the conditions so that the next one only gets evaluated
> after the first one fails you structure them like this.
> 
> ```
> ?condition1 {
>   statement when condition1 true
> } { 
>   ?condition2 {
>     statement when condition2 true
>   }
> }
> ```

### `Printing Text _________________________________ \n text <keyword>`

Text written inside of `" "` will be printed to the screen. Text needs to be
placed under a verb definition.

Newlines are considered whitespace and replaced with a space for correct flow
of the next line. You can force a newline by using `\n` in text.

Whitespace around newlines is ignored so you can indent text in the file.
You can use the escape character `\` to ignore that and force whitespace.

Reserved characters that need to be escaped with `\` in text are:

`" < > \ // __ == **`

Thus, you can use a '"' in text by escaping it '\"'. A combination of two
`/` is used for comments so you'll have to escape at least one if you want
to use them in text. Same for other two character tokens.

All other characters are legal and only treated as special characters
if outside of `" "` or within `< >`.

You can force a style on a line by placing it within pairs of style tokens:

- `**title**` - big, title font.
- `__quote__` - small, italic font.
- `==mono==` - small, mono-spaced font.

If the styled line doesn't start and end on a new line it will be added.
Styles cannot be nested or placed inside keywords (but keywords may be
placed within styled lines).

### `Keywords in Text ______________________________________ <keyword>`

Text can contain `<keywords>` inlined with text. Clicking on them creates a
menu with a list of possible actions based on the verbs that the user can
click on.

- `<keyword[noun]>` - keyword for the `[noun]`.
- `<keyword>` - keyword for the noun with the same name.
- `<keyword[noun:verb]>` - keyword which doesn't drop down a menu of available
                           actions but immediately executes the verb under
                           noun. This is called a choice.

There is one more type of keyword, used mostly internally:

`<keyword[noun=value:verb]>` - this sets the value of noun and then goes to
                               noun:verb. This is called value selection.

This is provided for convenience, you can get the same effect by moving the
assignment into the verb.

> `<keyword[noun=value]>` on its own is illegal as it would not visibly
> advance the story, giving no indication of an action having happened.

### `Special Noun Names ______________________________________ [QUEUE]`

These nouns are treated differently by the reader (not all are required):
`QUEUE, PLACE, EXITS, NOUNS, CALLS, QUICK`

These nouns expect string values of noun:verb that point to valid verbs to
execute:

- `[QUEUE]` - anything placed here will get executed and then removed
                automatically, this is used to kick-start the story. Any
                action taken by the player will be placed here and executed.
- `[CALLS]` - all values get executed every turn as instructions and
                remain here until removed. This happens prior to any action
                so the value of QUEUE is available to you if you need it.
- `[QUICK]` - like CALLS, executes every turn but is used to print the
                quick menu. It should not contain logic as it will only
                get executed if the quick menu exists in the reader.
                Values here don't need to be in NOUNS to remain active.

CALLS will not get executed on the turn it is added as it needs to happen
prior to an action. It can manipulate the QUEUE before it gets executed.
All values in QUEUE will get executed one by one and removed after each
execution. During execution QUEUE contains only the value currently being
executed.

> Remember that values can't store duplicate strings so you can't
> trigger multiple calls of the same noun:verb. Use CALLS if you want to
> keep calling the same value.

These nouns expect strings that match names of other valid nouns.

- `[PLACE]` - where we are.
- `[EXITS]` - where we can go.
- `[NOUNS]` - a noun must be here to remain active in the main pane after
                another action has taken place. Text from the previous action
                by default loses the keywords unless they are found here.

The absolute minimum is: `[QUEUE> = noun:verb]`. This will be called when
starting the story and will tell the reader to go to verb in noun and execute
that block.

### `Function Calls ___________________________________________ name()`

Function calls can also be used wherever a value can be used.

`!function(argument1, argument2)` - call function with the values  argument1
                                      and argument2.

The arguments are values so you can do execution inside the arguments so
both `function(arg1 + arg2)` and `function(@noun)` calls are legal.

You can even evaluate a name of the function `@noun1(@noun2)`. This will call
all the functions with names matching the text values of noun1 and use
values of noun2 as arguments for all the calls.

Currently available functions:

- `Play(asset)` - activate asset (returns true if played).
- `Stop(asset)` - deactivate asset (returns true if stopped).
- `Size(noun)` - return number of values of a noun (not counting the integer).
- `Print(value)` - print the value as plain text. If there are multiple
                   values print a comma separated list: `v1, v2`.
- `Keyword(value)` - print the value as a keyword. If there are multiple
                     values print a comma separated list: `<v1>, <v2>`.
- `SelectValue(noun:verb)` - print all noun values as value selections.
                             `<noun=v1:verb>, <noun=v2:verb>`.
- `Bookmark(value)` - silently create a bookmark at current time. A
                      noun:verb value is expected so it can be used to print
                      the description. If a bookmark is already present it
                      will not be overwritten.
- `Input(noun)` - will show an input box that will set the value of the noun.
                  The buttons on the input box will be the verbs of the noun.
                  If a number is entered, the integer value will be set in
                  addition to the text value. If the noun had a previous
                  value it will be shown in the input box.

Also available but mostly useful internally in the menu:

- `GetBooks()` - return book names of books in the book folder.
- `OpenBook(value)` - try and open a book of given name, return true if
                      successful.
- `CloseBook()` - close the book, save and return to the menu.
- `IsInGame()` - return true if a book is opened and a session started.
- `GetSnapshots(#number)` - return a list of snapshots up until #number as
                            text values and the end of range as the integer.
                            If no number or 0 passed in, it will return the
                            tail end of values. The number of values is
                            internally limited.
- `GetBookmarks(#number)` - Just like GetSnapshots, except with bookmarks.
- `GetSnapshotIndex()` - return the currently loaded snapshot index as the
                         integer value.
- `GetSessionName()` - return the session name currently loaded.
- `GetSessions(value)` - return user set names of sessions  for the given
                         book title or if no value given for the currently
                         book. If no value given and no is book open it will
                         return no values.
- `SaveSession(value)` - save the session to disk under a new name if
                         provided. This gets called for you automatically
                         when you close the book, try to branch, load or
                         exit the game altogether (except when force
                         quitting). Name will be made unique if needed.
- `BranchSession(value)` - save the session and create a new one that starts
                           at the current point in time. Use the provided
                           name if possible. It will default to a unique
                           name if a clashing one or none is provided.
- `NewSession()` - start a new session.
- `LoadSession(value)` - load a session with the given name, if no value is
                         given continue the last played session. Will start
                         a new session if no session is present.
- `UserBookmark(value)` - silently create a bookmark at current place using
                          the provided text value as description. The text
                          value can be provided by the player. This will
                          overwrite any previous bookmark at this place.
- `OpenMenu()` - open the game menu in the reader.
- `CloseMenu()` - close the game menu in the reader.
- `Quit()` - this will call CloseBook() to save the session and then quit
             the game.

### `Media Assets _________________________________________ $ = Type()`

Assets need to be defined before play starts.

`$asset=Type(parameter1, parameter2)` - creates an image, sound etc.

Asset definitions need to be outside noun definitions and start on a new
line. No linebreaks are allowed within definitions.

> These are not function calls and the syntax is different. You can't
> use nouns or do arithmetic on values, you need to use commas and parameter
> order is significant.

Each needs a unique name (asset names have their own namespace, separate
from nouns) and only one instance of each is allowed. Calling `Play(asset)`
whilst the asset is already active will do nothing. Calling Stop on a
stopped asset is safe and will do nothing.

Supported types are:

- `BG(filename)` - background, always to fit the window and behind all else.
- `Image(filename, X, Y, zoom, order)` - image float centered at (X, Y) where
                                         0 is in the centre and (-1, -1) is
                                         the upper left corner. Zoom is
                                         relative to the background so 1 will
                                         match its zoom. Order is lowest
                                         first and must be positive.
- `Music(filename)` - sound that loops.
- `Sound(filename)` - sound that plays once.
- `Voice(filename)` - sound that plays once and only one voice at a time.

