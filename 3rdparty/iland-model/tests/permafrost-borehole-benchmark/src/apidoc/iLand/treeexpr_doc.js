/**
A `TreeExpr` object is a helper object for efficient access to values of individual trees (>4m) in iLand.

The `TreeExpr` encapsulates an iLand expression (iland-model.org/Expression) which allows access to many properties of single trees in the model.
The basic use is demonstrated in the following example:

    // set up a TreeExpr
    var expr = new TreeExpr("dbh*dbh");

    // now use to evaluate the expression for individual trees, assume 'trees' is a TreeList
    var sum = 0;
    for (var i=0;i<trees.count;++i) {
        sum += expr.value(trees.tree(i));
    }

Similar functionality is provided by the {{#crossLink "Tree/expr:method"}}{{/crossLink}}-method of Tree - the difference is mainly performance: while the
{{#crossLink "Tree/expr:method"}}{{/crossLink}}-method constructs for each call an internal "expression" (and has to parse the content of the expression each time),
the `TreeExpr` object only parses the expression once.

Performance comparison
----------------------
The test case extracts the `dbh` of a tree 1,000,000 times in Javascript and measures the time (the overhead time of the Javascript loop (0.3s) is substracted)
* `dbh`-property (`tree.dbh`) (no Expression): 0.5s
* `TreeExpr` object (`expr.value(tree)`) (create Expression once): 1.1s
* `tree.expr("dbh")` (create Expression every time): 2.5s




@module iLand
@class TreeExpr
@constructor
*/

TreeExpr = {

/**
A string representing the current expression. The property is read/write and can be used to modify the expression.

@property expression
@type string
@example
    var expr = new TreeExpr("dbh");
    // assume 't' is a Tree:
    console.log(expr.value(t)); // e.g. 9.799 cm
    expr.expression = "age";
    console.log(expr.value(t)); // e.g. 21 yrs
*/


/**
Constructs a `TreeExpr`. Use with the `new` JavaScript keyword.

@param string expression The expression as a string
@method TreeExpr
*/

/**
Calculates the value of the expression for the given `tree`. If the tree is invalid, a warning is written to the log and -1 is returned.

@param {Tree} tree The {{#crossLink "Tree"}}{{/crossLink}}-object for which to calculate the expression.
@method value
*/
}
