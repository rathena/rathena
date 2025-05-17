
### Running the examples ###

Expected Console Output:

```C++
<<< Example 1 >>>

'longitude' setting is out of valid bounds (max: 180). Value was: 1200.35
Missing 'latitude' setting in configuration file.
Cannot proceed until all mandatory settings are set.


<<< Example 2 >>>

TITLE                           AUTHOR                           PRICE   QTY
Treasure Island                 Robert Louis Stevenson          $ 29.99  5
Snow Crash                      Neal Stephenson                 $  9.99  8


<<< Example 3 >>>

'longitude' setting is out of valid bounds (max: 180). Value was: 1200.35
Missing 'latitude' setting in configuration file.
Cannot proceed until all mandatory settings are set.

Expected Config Layout:

// -- begin --
name = "<name>";
abstract = "<unknown>";
longitude = 0.0;
latitude = 0.0;
inventory =
{
   movies = ( );
   books = (
      {
         title = "bookXYZ";
      } );
};
// --- end ---
```