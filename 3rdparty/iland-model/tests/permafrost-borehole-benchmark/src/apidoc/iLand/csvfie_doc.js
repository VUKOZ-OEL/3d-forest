/**
The `CSVFile` is a helper class to load and process tabular data which is stored in a text file.

The type of delimiter is auto-detected from the first lines of the file. Possible delimeters are `;`, `,`, a tabulator (\t) or space(s).

Lines beginning with `#` are and empty lines are ignored. Quotes around (`"`) values are removed. Both Linux (\n) and Windows (\r\n) line ending characters are supported.

All indices are 0-based (e.g., valid rows are 0..rowCount-1).

Example
-------

        var csv = Factory.newCSVFile(""); // do not load a file

        csv.loadFromString('a;b;c\n1;2;3\n1;4;12.1'); // note the newline characters
        console.log( csv.value(1,2) ); // -> 12.1

        // change a value
        csv.setValue(csv.rowCount-1,csv.columnIndex('a'), 100);
        csv.saveFile(Globals.path('temp/test.csv'));

        var csv = Factory.newCSVFile( Globals.path('temp/test.csv') );
        console.log( csv.value(1,"c") ); // -> 12.1
        console.log( csv.row(0) ); // get the full row (including delimiters)

        // loop over all rows
        for (var i=0;i<csv.rowCount;++i)
            console.log( csv.value(i,0) );


 @module iLand
 @class CSVFile
 */

CSVFile = {

    // properties


    /**
        if `true`, then the first line of the input file are considered to be headers. Default: `true`.

        See also: {{#crossLink "CSVFile/columnIndex:method"}}{{/crossLink}}

        @property captions
        @type bool
    */
    /**
        if `true`, it is assumed that the file contains only one column (default: false).

        @property flat
        @type bool
    */
    /**
        The number of columns in the table (-1 if not loaded).

        @property colCount
        @type integer
    */
    /**
        The number of rows in the table (-1 if not loaded).

        @property rowCount
        @type integer
    */
    // methods


    /**
    Load a table from the file given in `fileName`. The delimiter is autodetected (see above).

    See also: {{#crossLink "Factory/newCSVFile:method"}}{{/crossLink}}

    @method loadFile
    @param {string} fileName file to load (Use {{#crossLink "Globals/path:method"}}{{/crossLink}} for using relative paths).
    @return { bool } `true` on sucess
    */

    /**
    Load a table from a string. The delimiter is autodetected (see above). Use newline characters (`\n`) to separate lines. Captions

    See also: {{#crossLink "Factory/newCSVFile:method"}}{{/crossLink}}

    @method loadFromString
    @param {string} content  string that should be interpreted as a table.
    @return {bool} `true` on sucess
    */
    /**
    Retrieve the column name with the index `col`.

    @method columnName
    @param {integer} col  a valid column index (0<=index<colCount).
    @return {string} the column name
    */
    /**
    Retrieve the index of the column with the name `colName`. The index can be used in later call to `value()`. The functions returns -1 if the column is not present.

    @method columnIndex
    @param {string} colName a column name
    @return {integer} the index of the given column, or -1 if the column is not available.
    */
    /**
    Retrieve the value of the column `colName`and the row with the index `row`.

    @method value
    @param {integer} row  row index of the value to retrieve (0<=row<rowCount).
    @param {string} colName  a column name
    @return {string} the value (use Javascript's `Number()` to convert to a number)
    */
    /**
    Retrieve the value of the column with the index `col`and the row with the index `row`. The return value is a string value. The javascript built-in function `Number` can be used to force a conversion to a numerical value.

    See also: {{#crossLink "CSVFile/columnIndex:method"}}{{/crossLink}},{{#crossLink "CSVFile/jsValue:method"}}{{/crossLink}}

    @method value
    @param {integer} row  row index of the value to retrieve (0<=row<rowCount).
    @param {integer} col column index of the requested value (0<=col<colCount).
    @return {string} the value (use Javascript's `Number()` to convert to a number)
    */
    /**
    Retrieve the value of the column `colName`and the row with the index `row`.

    @method jsValue
    @param {integer} row  row index of the value to retrieve (0<=row<rowCount).
    @param {string} colName  a column name
    @return {value} the value (either int, double, or string converted from the input)
    */
    /**
    Retrieve the value of the column with the index `col`and the row with the index `row`. The return value is a string value. The javascript built-in function `Number` can be used to force a conversion to a numerical value.

    See also: {{#crossLink "CSVFile/columnIndex:method"}}{{/crossLink}},{{#crossLink "CSVFile/value:method"}}{{/crossLink}}

    @method jsValue
    @param {integer} row  row index of the value to retrieve (0<=row<rowCount).
    @param {integer} col column index of the requested value (0<=col<colCount).
    @return {value} the value as a Javascript value (int, double, or string)
    */

    /**
    Change the value of the cell with in the column with the index `col`and the row with the index `row`. The new value can be either a numeric or a string.

    See also: {{#crossLink "CSVFile/value:method"}}{{/crossLink}}

    @method setValue
    @param {integer} row index of the value to retrieve (0<=row<rowCount).
    @param {integer} col column index of the requested value (0<=col<colCount).
    @param {variant} newValue  new value (either a string or numeric).
    */
    /**
    Save the current state of the table to a file with the name `fileName`.

    See also: {{#crossLink "CSVFile/loadFile:method"}}{{/crossLink}}, {{#crossLink "CSVFile/loadFromString:method"}}{{/crossLink}}

    @method saveFile
    @param {string} fileName file to be created or overwritten (Use {{#crossLink "Globals/path:method"}}{{/crossLink}} for using relative paths).
    @return { bool } `true` on sucess
    */

}
