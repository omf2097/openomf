Sprite Packer Algorithm
=======================

The sprite packer implements a bin-packing algorithm for allocating rectangular regions
within a fixed-size texture atlas. It efficiently packs sprites of varying sizes while
minimizing wasted space.

The packer maintains a list of free rectangular regions. When a sprite needs to be placed,
the algorithm finds a suitable free region, allocates space for the sprite, and splits the
remaining space into new free regions.

Allocation Algorithm
--------------------

When ``sprite_packer_alloc()`` is called with a requested width and height:

1. **Best-Fit Search**
   Search the free space list for the smallest region that can fit the requested dimensions.
   The search uses perimeter as the size metric (smaller perimeter = better fit).

2. **Seek Limit**
   To balance quality vs. speed, the search stops after finding 10 candidates once a valid
   region is found. This prevents exhaustive searches in heavily fragmented atlases.

3. **Region Splitting**
   After allocation, the remaining space is split into up to two new regions.

4. **List Maintenance**
   The free list is kept sorted by size (largest first) for efficient searching.

.. graphviz::

   digraph allocation {
       rankdir=TB
       node [shape=box, fontname="monospace"]

       start [label="sprite_packer_alloc(w, h)", shape=ellipse]
       search [label="Search free list"]
       found [label="Region found?", shape=diamond]
       fail [label="bail out", shape=ellipse]
       exact [label="Exact fit?", shape=diamond]
       delete [label="Delete region\nfrom free list"]
       split [label="Split region into\nFound, A and B"]
       update [label="Update free list\nwith A and/or B"]
       sort [label="Sort free list\n(largest first)"]
       success [label="Return found position", shape=ellipse]

       start -> search -> found
       found -> fail [label="no"]
       found -> exact [label="yes"]
       exact -> delete [label="yes"]
       exact -> split [label="no"]
       delete -> success
       split -> update -> sort -> success
   }

Region Splitting Strategy
-------------------------

When a sprite is placed in a larger free region, the remaining L-shaped space is split
into two rectangles. The split direction depends on which remaining dimension is longer,
minimizing fragmentation.

**Case 1**

Split so B gets the full width (preserves the larger bottom region):

.. graphviz::

   digraph split_horizontal {
       node [shape=plaintext]
       graph [rankdir=LR]

       before [label=<
           <table border="1" cellborder="1" cellspacing="0">
               <tr><td bgcolor="lightblue" width="100" height="100">Free Region<br/>100 x 100</td></tr>
           </table>
       >]

       arrow [label="  →  ", shape=plaintext]

       after [label=<
           <table border="0" cellborder="1" cellspacing="0">
               <tr>
                   <td bgcolor="gray" width="60" height="30">Sprite<br/>60 x 30</td>
                   <td bgcolor="lightgreen" width="40" height="30">A<br/>40 x 30</td>
               </tr>
               <tr>
                   <td colspan="2" bgcolor="lightyellow" width="100" height="70">B (full width)<br/>100 x 70</td>
               </tr>
           </table>
       >]

       before -> arrow -> after [style=invis]
   }

**Case 2**

Split so A gets the full height (preserves the larger right region):

.. graphviz::

   digraph split_vertical {
       node [shape=plaintext]
       graph [rankdir=LR]

       before [label=<
           <table border="1" cellborder="1" cellspacing="0">
               <tr><td bgcolor="lightblue" width="100" height="60">Free Region<br/>100 x 60</td></tr>
           </table>
       >]

       arrow [label="  →  ", shape=plaintext]

       after [label=<
           <table border="0" cellborder="1" cellspacing="0">
               <tr>
                   <td bgcolor="gray" width="40" height="30">Sprite<br/>40 x 30</td>
                   <td rowspan="2" bgcolor="lightgreen" width="60" height="60">A (full height)<br/>60 x 60</td>
               </tr>
               <tr>
                   <td bgcolor="lightyellow" width="40" height="30">B<br/>40 x 30</td>
               </tr>
           </table>
       >]

       before -> arrow -> after [style=invis]
   }

Free List Management
--------------------

The free list is maintained as a sorted vector:

1. **Sorting**: Regions are sorted by size (w + h) in descending order, so larger
   regions appear first.

2. **Search Direction**: The search iterates backwards (smallest to largest) to find
   the best fit efficiently.

3. **Zero-Area Regions**: After splitting, any region with zero area (w=0 or h=0)
   is discarded rather than added to the free list.

Example Packing Sequence
------------------------

This example demonstrates packing three sprites into a 256×256 atlas.

**Step 1: Initial State**

The packer starts with one free region covering the entire 256×256 area.

.. graphviz::

   digraph step0 {
       node [shape=plaintext]
       atlas [label=<
           <table border="1" cellborder="0" cellspacing="0">
               <tr><td bgcolor="lightblue" width="100" height="100">Free<br/>256×256</td></tr>
           </table>
       >]
   }

Free list: ``[(0,0) 256×256]``

**Step 2: Allocate Sprite A (100×100)**

The 100x100 region fits, since this is the first allocation. For the remaining space, case 2 applies.

.. graphviz::

   digraph step1 {
       node [shape=plaintext]
       atlas [label=<
           <table border="0" cellborder="1" cellspacing="0">
               <tr>
                   <td bgcolor="coral" width="40" height="40">A<br/>100×100</td>
                   <td rowspan="2" bgcolor="lightblue" width="60" height="100">Free<br/>156×256</td>
               </tr>
               <tr>
                   <td bgcolor="lightblue" width="40" height="60">Free<br/>100×156</td>
               </tr>
           </table>
       >]
   }

Free list: ``[(100,0) 156×256, (0,100) 100×156]``

**Step 3: Allocate Sprite B (80×60)**

Best fit is the 100×156 region (smaller than 156×256). For the remaining space, case 1 applies.

.. graphviz::

   digraph step2 {
       node [shape=plaintext]
       atlas [label=<
           <table border="0" cellborder="1" cellspacing="0">
               <tr>
                   <td colspan="2" bgcolor="coral" width="40" height="40">A<br/>100×100</td>
                   <td rowspan="3" bgcolor="lightblue" width="60" height="100">Free<br/>156×256</td>
               </tr>
               <tr>
                   <td bgcolor="gold" width="32" height="24">B<br/>80×60</td>
                   <td bgcolor="lightblue" width="8" height="24">Free<br/>20×60</td>
               </tr>
               <tr>
                   <td colspan="2" bgcolor="lightblue" width="40" height="36">Free<br/>100×96</td>
               </tr>
           </table>
       >]
   }

Free list: ``[(100,0) 156×256, (0,160) 100×96, (80,100) 20×60]``

**Step 4: Allocate Sprite C (100×80)**

This sprite is too wide for the 20×60 region, but fits in 100×96 (the bottom-left
area). Since the width is an exact match, only region B is created.

.. graphviz::

   digraph step3 {
       node [shape=plaintext]
       atlas [label=<
           <table border="0" cellborder="1" cellspacing="0">
               <tr>
                   <td colspan="2" bgcolor="coral" width="40" height="40">A<br/>100×100</td>
                   <td rowspan="4" bgcolor="lightblue" width="60" height="100">Free<br/>156×256</td>
               </tr>
               <tr>
                   <td bgcolor="gold" width="32" height="24">B<br/>80×60</td>
                   <td bgcolor="lightblue" width="8" height="24">Free<br/>20×60</td>
               </tr>
               <tr>
                   <td colspan="2" bgcolor="orchid" width="40" height="32">C<br/>100×80</td>
               </tr>
               <tr>
                   <td colspan="2" bgcolor="lightblue" width="40" height="6">Free<br/>100×16</td>
               </tr>
           </table>
       >]
   }

The atlas now contains three sprites with the large 156×256 region on the right still
available for future allocations.

Free list: ``[(100,0) 156×256]``
