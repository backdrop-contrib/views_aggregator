VIEWS AGGREGATOR PLUS
=====================
Because the Views and ViewsCalc modules rely on the database to perform
aggregation, you only have limited options at your disposal. That is where this
module comes in. Unlike Views and ViewsCalc, this module:
- enumerates group members (see https://drupal.org/node/1300900)
- produces tallies (textual histograms, see http://drupal.org/node/1256716)
- can aggregate on Views PHP code-snippets (note: Views PHP has not been ported to Backdrop, and it may never be given the security risks it presents)
- can filter out rows on regular expressions (regexp)
- can aggregate across entire columns (e.g show column data range at the top)
- lets you add your own custom aggregation functions to the existing set
- aggregation functions can take parameters, as currently employed by "Filter
  rows", "Count" and "Label"
- on Views of type "Webform submissions" the module supports the field "Webform
  submission data: Value"

Basics Recap: what is aggregation again?
----------------------------------------
In the context of Views and this module, aggregation is the process of grouping
and collapsing result rows on the identical values of ONE column, while at the
same time applying "summary" functions on other columns. For example you can
group the result set on a taxonomy term, so that all rows sharing the same
value of the taxonomy column are represented as single rows, with aggregation
functions, like TALLY, SUM, or ENUMERATE applied to the remaining columns.

Example
-------
Say the original View based on raw database results looks like below.

Industry|Company Name |     Turnover |
--------|-------------|--------------|
IT      |       AquiB |  $25,000,000 |
Clothing|    Cenneton |  $99,000,000 |
Food    |       Heiny |  $66,000,000 |
IT      |PreviousBest |  $ 5,000,000 |
Food    |   McRonalds | $500,000,000 |

Then with the grouping taking place on, say Industry, and aggregation functions
COUNT and SUM applied on Company Name and Turnover respectively, the final
result will display like below. A descending sort was applied to
Turnover and the display name of "Company Name" was changed to "Comp. Count".

Industry| Comp. Count |     Turnover |
--------|-------------|--------------|
Food    |           2 | $566,000,000 |
Clothing|           1 |  $99,000,000 |
IT      |           2 |  $30,000,000 |

That's the basics and you can do the above with Views. But with Views
Aggregator Plus (VAgg+) you can also aggregate like below, using its TALLY and
ENUMERATE group aggregation functions, as well as LABEL, COUNT and SUM for the
added bottom row.

Industry    |Companies           |     Turnover |
------------|--------------------|--------------|
Food (2)    |Heiny, McRonalds    | $566,000,000 |
Clothing (1)|Cenneton            |  $99,000,000 |
IT (2)      |AcquiB, PreviousBest|  $30,000,000 |
------------|--------------------|--------------|
Totals      |                  5 | $695,000,000 |
------------------------------------------------


HOW TO USE
----------
On the main Views UI page, admin/structure/views/view/YOUR-VIEW/edit/page,
under Format, click and select "Table with aggregation options". Having arrived
at the Settings page, follow the hints under the header "Style Options".
All group aggregation functions, except "Filter rows" require exactly one field
to be assigned the "Group and compress" function.
Column aggregation functions may be used independently of group aggregation
functions. If a column aggregation function requires an argument, it may take
it from the corresponding group aggregation function, if also enabled.

There are no permissions or global module configurations.

Views Aggregator Plus does not combine well with Views' native aggregation.
So in the Advanced section (upper right) set "Use aggregation: No".

Keep in mind that the process of grouping and aggregation as performed by this
module is different from the Grouping option in Views. With Grouping in Views
the total number of rows remains the same, but the rows are grouped in separate
tables. With this module, the number of rows is reduced as they are grouped and
collapsed, but the end result is always a single table.

FUNCTION PARAMETERS
-------------------
Functions marked with an asterisk take an optional parameter.

"Group and Compress" takes an optional keyword 'case-insensitive' (in English or
in the translated language on your site) to perform grouping in a
case-insensitive manner. The default is case-sensitive.

"Average" takes an optional precision: the number of decimals to round to after
calculating the average.

"Range", "Tally members" and the two "Enumerate" functions use their parameter
to specify the separator. The default is an HTML line-break, `<br/>`, for "Tally"
and "Enumerate" and ' - ' for "Range".

"Filter rows" and "Count" take a regular expression. This is explained below.

REGEXPS
-------
Some aggregation functions, like "Filter rows" and "Count" take a regular
expression as a parameter. In its simplest form a regular expression is a word
or part of a word you want to filter on. If you use regexps in this way, you may
omit the special delimiters around the parameter, most commonly a pair of
forward slashes. So "red" and "/red/" are equivalent.
Here are some more regexps:

Expression       |  Result                                                                           |
---------------- |-----------------------------------------------------------------------------------|
\/RED\/i         |  targets rows that contain the word "red" in the field specified, case-insensitive|
\/red\\|blue\/   |  rows with either the word "red" or "blue" in the field                           |
\/^(Red\|Blue)\/ | rows where the specified field begins with "Red" or "Blue"                        |
\/Z[0-9]+\/      |  the letter Z followed by one or more digits                                      |

Ref: http://work.lauralemay.com/samples/perl.html (for PERL, but quite good)

LIMITATIONS
-----------
- If an aggregation function result does not display correctly, try changing the
  field formatter. For example use "Plain text", rather than "Default".
- Views-style table grouping, whereby the original table is split into smaller
  ones, interferes with this plugin, so is not available.
- When you have an aggregated View AND a normal View attachment on the same
  page AND you click-sort on Global:Math Expression the normal View attachment
  will temporarily disappear. This is because the sort is passed to BOTH
  displays and normal Views do not support sorting on Math Expressions.
- When you apply two aggregation functions on the same field, the 2nd function
  gets applied on the results of the first -- not always what you want.
- Grouping, tallying and other functions may not work correctly when you enable
  the "Theme debug" option provided by the Devel module.


ACKNOWLEDGMENT
--------------
The UI of this module borrows heavily from Views Calc and the work by the
authors and contributors done on that module is gratefully acknowledged.

REFs
----
- https://drupal.org/node/1219356#comment-4782582
- https://drupal.org/node/1219356#comment-6909160
- https://drupal.org/node/1300900
- https://drupal.org/node/1791796
- https://drupal.org/node/1140896#comment-7657061

CREDITS
-------
- Current Backport version maintainer: [argiepiano](https://github.com/argiepiano)  
- Original Drupal module supporting organizations:
  - [flink](https://www.drupal.org/flink) all development and support
