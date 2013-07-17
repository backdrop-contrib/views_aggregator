
VIEWS AGGREGATOR PLUS
=====================
Because the Views and ViewsCalc modules rely on the database to execute
aggregation, you only have limited options for aggregation. As the great Merlin
said himself: "You can't aggregate a PHP expression in the database. :/"
[http://drupal.org/node/1219356#comment-4736826]
This is where this module comes in. Unlike Views and ViewsCalc, this module:
o enumerates group members (see https://drupal.org/node/1300900)
o produces tallies (textual histograms, see http://drupal.org/node/1256716)
o can aggregate on ViewsPHP expressions
o can return the most or least frequently occurring member
o lets you add your own custom aggregation functions to the existing set

Basics Recap: what is aggregation again?
----------------------------------------
In the context of Views and this module, aggregation is the process of grouping
and collapsing result rows on the identical values of ONE column, while at the
same time applying "summary" functions on other columns. For example you can
group the result set on a taxonomy term, so that all rows sharing the same
value of the taxonomy column are represented as single rows, with aggregation
functions, like COUNT, SUM, or ENUMERATE applied to the remaining columns.

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
result will display like so:

Industry| Comp. Count |     Turnover |
--------|-------------|--------------|
Food    |           2 | $566,000,000 |
IT      |           2 |  $30,000,000 |
Clothing|           1 |  $99,000,000 |

That's the basics and you can do the above with Views. But with Views
Aggregator Plus you can also aggregate like so:

Industry    |Companies           |     Turnover |
------------|--------------------|--------------|
Food (2)    |Heiny, McRonalds    | $566,000,000 |
IT (2)      |AcquiB, PreviousBest|  $30,000,000 |
Clothing (1)|Cenneton            |  $99,000,000 |

But that's just the beginning. Don't forget you can aggregate on ViewsPHP
expressions, so the possibilities are endless! Say you have a content type
"event" that has a date range field on it with both start and end components
active. Let's say its machine name is "field_duration". Then code snippet below
entered in the "Output" area of a Views PHP field will output in Views for each
event whether it is in progress, closed or not started yet.

<?php
  $start_date = strtotime($data->field_field_duration[0]['raw']['value']);
  $end_date   = strtotime($data->field_field_duration[0]['raw']['value2']);
  echo time () < $start_date ? 'not started' : (time() < $end_date ? 'in progress' : 'closed');
?>

Next you can use VAgg+ to group on the expression and count or enumerate the
event titles in each of these categories.

NOTES
-----
Most of Views tabular output functions have been replicated in this plugin,
even when Views PHP is used:
o Column sorting and token support seem mostly ok.
o CSS styling options (striping, table and row classes) work.
o Combining the output of multiple columns and putting it in another works ok.
o Views-style table grouping, which different from aggregation, interferes with
  this plugin, so is not available.
o When you apply two aggregation functions on the same field, the 2nd function
  gets applied on the results of the first -- normally not what you want
o For technical reasons, when enumerating group members, hyperlink markup is
  automatically dropped; the members appear in plain text.

POSSIBLE ENHANCEMENTS
---------------------
o Handle images (e.g. as enumerations)

TIPS FOR USING VIEWS PHP MODULE
-------------------------------
Use "Output expressions", not "Value expressions", as in the "Value" field few
query results are available.



FAQ's
-----
Q: With Views Aggregator Plus installed, is there a reason to use the Views
   aggregation options?
A: VAgg+ has all the standard Views aggregation functions plus more. For huge
   datasets the native db functions employed by Views are possibly a little
   faster in execution than VAgg+'s post-query approach.

Q: As a developer can I add more aggregation functions in a maintainable
   fashion and without hacking 
Y: You can write your own mini-module for this in which you implement
   hook_views_aggregation_functions_info(). It's easy. See how it was done
   for this module. See file views/views_aggregator_functions.inc

Q: Is Views Aggregator Plus different from ViewsCalc?
A: Yes. ViewsCalc aggregates entire columns (and individual fields), while
   Views Aggregator Plus aggregates on groups within columns.
   ViewsCalc shows the aggregated results at the bottom of the original table,
   whereas Views Aggregator Plus reduces the number of rows and shows the
   aggregated results within the table, very much like Views' native aggregation
   does. However VAgg+ has more aggregation functions and new ones can be added.

Q: Can I use Views Aggregator Plus with ViewsCalc?
A: If is safe to enable both modules. However each offers its own Views table
   style format, so you can't create tabular output that features the combined
   results of both on the same page (patches anyone?).



https://drupal.org/node/1219356#comment-4782582
https://drupal.org/node/1219356#comment-6909160
https://drupal.org/node/1300900
https://drupal.org/node/1791796

