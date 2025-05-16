# Roadmap

Roughly in order of priority:

  * Cleanup:
    * Review & cleanup API surface.
    * Turn calls to `C4_ASSERT()` into calls to `RYML_ASSERT()`
  * Add emit formatting controls:
    * add single-line flow formatter
    * add multi-line flow formatters
      * indenting
      * non indenting
    * keep current block formatter
    * add customizable linebreak limits (number of columns) to every formatter
    * add per node format flags
    * (lesser priority) add auto formatter using reasonable heuristics to
      switch between other existing formatters
  * Investigate possibility of comment-preserving roundtrips
