Layout of the VST.
-------------------------------------------------------------------------------
|                                          |---------------------------------||
|       Upload File  \|/                   | 1                               ||
|                                          | 2                               ||
|                                          | 3                               ||
|  Prompt_________________________________ | 4                               ||
|                                          | 5                               ||
|  Instrumentation________________________ | 6                               ||
|                                          | 7                               ||
|  length [xx.xx] BPM [xxx.xx] Samples [x] | 8                               ||
|                                          | 9                               ||
|  --------------------------------------- | 10                              ||
|  |              Generate               | |---------------------------------||
|  ---------------------------------------                                    |
|                                                                             |
|  Temperature   Top K     Pop P   CFG                                        |
|      Dial       Dial      Dial   Dial                                       |
|     Float       Float    Float   Int                                        |
|      0-2       1-1000    0-1     1-10                                       |
|                                                                             |
-------------------------------------------------------------------------------

For your idea:
The dials are collapsable, so the window expands in height to open them.
The dials are for the AI, do make sure to check the MusicGen folder for the actual ranges.
The things in between [] are number boxes, make sure to adhere to the comma position, also when there is no comma (or I mean .) then it is an integer.
Samples cannot be 0, max 10
BPM cannot be 0, max 300
Length cannot be 0, max 30

The Prompt and instrumentation are input boxes.
The Upload File button is a regular JUCE button which opens a context window for the user to select an audio file to give to the AI model.

The generate button is just a button.