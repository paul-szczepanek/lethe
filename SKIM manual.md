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

Logic syntax is enclosed in square `[ ]` brackets. Keyword syntax is enclosed
in angled `< >` brackets. Whitespace is ignored and may be used for
indentation to aid readability.

> Comments like this one may mention things that are explained fully in
> subsequent sections.

---

### `Noun Definition ___________________________________________ [<>] [<[]>]`

`[<noun>]` - begin definition of the noun. This delimits noun definitions and
             needs to start on a new line.

Nouns can't start with numbers (`1noun` would be an illegal name) but may
contain numbers and spaces. Leading and trailing spaces will be ignored. Use
alphanumeric characters. Nouns need to have unique names.

> Nouns are Case **Sensitive**!
> Keep that in mind if you start a sentence with a keyword and use a capital
> letter and the noun uses a lower case letter (use `<Keyword[noun]>` then)

`[<noun[pattern]>]` - defines a noun using a pattern.

Patterns get prepended to the definition and every occurrence of the pattern
string gets replaced with the noun string of the noun being defined.

`[<[pattern]>]` - this defines a pattern for nouns to use.

They are not evaluated and need to be defined before they are used.
They are essentially macros that get prepended to a noun definition.

### `Values ________________________________________________________ [<> = ]`

`[<noun> = value1, value2, #9]` - sets the initial values of the noun.

Prepend integers with `#`. If no `#` is used then the number is treated as
text.

There can only ever be one integer value. `[<noun> = #1, #7, #8]` will set the
integer value to 8.

All nouns have an integer value 0 until it's changed. The integer value
doesn't count as a regular text value.

> In the initial assignment, you can't force noun evaluations with @.

### `Verb Definition ___________________________________________________ [:]`

Verbs define the actions possible in the keyword menu.

- `[:verb]` - past this is where execution goes when you choose the action.
- `[:verb1] [:verb2]` - only the first one is visible in the drop down menu
                        but both work (internal alias).
- `[:] [:hidden verb]` - leaving the name of the first verb blank will make
                         the action hidden from the list.

You can have multiple verb definitions of the same name. First one with true
top level condition will activate upon being called.

### `Conditions ________________________________________________________ [?]`

Value comparisons:

- `[?value1 += value2]` - does value1 contain value2.
- `[?value1 -= value2]` - does value1 not contain value2.
- `[?value1 = value2]` - does value1 contain value2 and nothing else.

If you omit the left side of the condition, it is implied that you mean the
values that the noun currently being defined contains.

`[?= value1]` - does the noun being defined contain value1.

> An implied noun will expand to `@noun` and/or `#noun` based on the contents
> of the expression on the right.

These will try to compare the integer value of a:

`[?#a = #4]` `[?#a <= #5]` `[?#a >= #2]` `[?#a < #4]` `[?#a > #8]`

If no comparison is present it will return true if it has any contents.

`[?value]` - is true if integer greater than 0 or has text values.

Conditions may be placed above verb definitions. They will work by
hiding the verb if the condition is not met.

### `Instructions ______________________________________________________ [!]`

`[!b:verb]` - go to and execute `[b:verb]` (then return and continue).

Assignment instructions:

- `[!noun += value]` - add value to noun.
- `[!noun -= value]` - remove value from noun.
- `[!noun = value]` - set values of noun to value (removes previous values).
- `[!= value]` - missing noun defaults to the one being defined.
- `[!noun=]` - delete all values of noun (except the integer value).
- `[!noun = #0]` - set the integer value of noun to 0.

What is on the left side is the name of the noun you want to assign to.

> Using @ might not work as you expect as @noun will evaluate to its values.
> `[!@noun = value]` will not assign value to noun. It will assign value to
> nouns with names that match existing values of noun.

Instructions need to be below a verb definition.

### `Evaluation ________________________________________________________ @ #`

Rather than using value literals (numbers and strings) for an instruction or
condition you can get the values contained in a noun. `#noun` evaluates to the
integer value and `@noun` to string values of the noun.

- `[!noun = string]` - assign the value string to noun.
- `[!noun1 = @noun2]` - assign the text values of noun2 to noun1
- `[!noun1 = #noun2]` - assign the integer value of noun1 the integer value
                        of noun2.

> If letters follow `#` than rather than a integer literal it's assumed that
> what follows is a noun name (nouns can't start with digits).

Numbers are handled wholly separate so if you wanted to copy both strings
and the integer value from noun2 to noun1 you would need to explicitly say:

`[!noun1 = @noun2 + #noun2]`

You can  use evaluations in instructions and conditions.

`[!@noun1:@noun2]` - this will go to noun1_value:noun2_value. If there are
                     multiple values it will execute them all in sequence.

You can nest @ evaluations.

`[!@@noun1:verb]` - will go to noun3:verb, if noun1 has a value called noun2
                    and there is a noun2 that has a value called noun3.

### `Arithmetic ________________________________________________ ( ) - + * /`

You can do arithmetic operations on numbers with `+ - / * ( )`.

You can also add and subtract text values. Multiplying text values
concatenates them. Dividing text values returns values present in both
values.

Let's assume we have two nouns defined like so:

- `[<noun1> = A, C]`
- `[<noun2> = B, C]`

Then the following statements are all true:

- `[?@noun1 = A + C]`
- `[?@noun1 * @noun2 = AB + AC + CB + CC]`
- `[?@noun1 / @noun2 = C]`

Order of operations is left to right regardless of type so if you need
a different order of evaluation you can use parenthesis `( )`.

### `Code Blocks _____________________________________________ { } & | [!<<]`

`{ }` are used for blocks. If no `{ }` are used then the code block is assumed
to end at the next condition. Implied block of a top level condition
(condition above a verb) ends with the next top level condition.

All the braces `{ }` below can be omitted without changing the meaning.

```
[<noun>]
[?top condition1] {
  [:verb1]
    [?condition1] {
      text
    }
    [?condition2] {
      text
    }
}
[?top condition2] {
  [:verb2]
    text
}
```

You can use `&` and `|` to create series of conditions or instructions:

- `[] & []` - will short circuit, first negative result stops.
- `[] | []` - will short circuit, first positive result stops.

> Don't mix `!` and `?` within these series, results are undefined.

Execution will continue to the end of the brace and return to the parent block
unless a break instruction `[!<<]` is encountered.

- `[!<<]` - breaks out of the current and parent block.
- `[!<<<]` - breaks up three levels up.
- ...and so on, add more `<` for more levels.

> Note that a single `<` as in `[!<]` is illegal (and would be pointless).

### `Printing Text _______________________________________ \n text <keyword>`

Text written outside of `[ ]` will be printed to the screen. Text needs to be
placed under a verb definition.

Text can contain `<keywords>` inlined with text. Clicking on them creates a
menu with a list of possible actions based on the verbs that the user can
click on.

- `<keyword[noun]>` - keyword for the `[<noun>]`.
- `<keyword>` - keyword for the noun with the same name.
- `<keyword[noun:verb]>` - keyword which doesn't drop down a menu of available
                           actions but immediately executes the verb under
                           noun. These are called choices.

Newlines are considered whitespace and replaced with a space for correct flow
of the next line. You can force a newline by using `\n` in text.

Whitespace around newlines is ignored so you can indent text in the file.
You can use the escape character `\` to ignore that and force whitespace.

Reserved characters that need to be escaped with `\` in text are:

`< > [ ] { } \ & | //`

`&` and `|` will work unescaped but will contract whitespace around them
(changing `R & R` to `R&R`). A combination of two `/` is used for comments so
you'll have to escape one if you want to use them in text.

All other characters are legal and only treated as special characters
if within `[ ]`.

### `Special Noun Names __________________________________________ [<BEGIN>]`

These nouns are treated differently by the reader (not all are required):
`BEGIN, PLACE, EXITS, NOUNS, CALLS, QUICK`

These nouns expect string values of noun:verb that point to valid verbs to execute:

- `[<BEGIN>]` - how the story starts (this is essential).
- `[<CALLS>]` - all values get executed every turn as instructions.
- `[<QUICK>]` - like CALLS, executes every turn but is used to print the
                quick menu. It should not contain logic as it will only
                get executed if the quickmenu exists in the reader.
                Values here don't need to be in NOUNS to remain clickable.

These nouns expect strings that match names of other valid nouns.

- `[<PLACE>]` - where we are.
- `[<EXITS>]` - where we can go.
- `[<NOUNS>]` - a noun must be here to remain clickable in the main pane.
                Text from the previous action will lose keywords unless they
                are here.

The absolute minimum is: `[<BEGIN> = noun:verb]`. This will be called when
starting the story and will tell the reader to go to verb in noun and execute
that block.

### `Function Calls _________________________________________________ name()`

Function calls can also be used wherever a value can be used.

`[!function(argument1, argument2)]` - call function with the values  argument1
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
- `Print(noun)` - print the noun as a keyword. If there are multiple
                  values print a comma separated list: `<v1>, <v2>`.

### `Media Assets _____________________________________________ [$ = type()]`

Assets need to be defined before play starts.

`[$asset=type(parameter1, parameter2)]` - creates an image, sound etc.

Asset definitions need to be outside noun definitions and start on a new
line. No linebreaks are allowed within definitions.

> These are not function calls and the syntax is different. You can't
> use nouns or arithmetic on values, you need to use commas and parameter
> order is significant

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

