..
   ****************************************************************************
    pgRouting Manual
    Copyright(c) pgRouting Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: http://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************

pgr_bipartite -Experimental
===============================================================================

``pgr_bipartite`` — If graph is bipartite then function returns the vertex id along with color (0 and 1) else it will return an empty set.
In particular, the is_bipartite() algorithm implemented by Boost.Graph.

.. figure:: images/boost-inside.jpeg
   :target: https://www.boost.org/doc/libs/1_53_0/libs/graph/doc/is_bipartite.html

   Boost Graph Inside

.. include:: experimental.rst
   :start-after: begin-warn-expr
   :end-before: end-warn-expr

.. rubric:: Availability

* Version 3.2.0

  * New **experimental** function

.. rubric:: Support

* **Supported versions:**
  current(`3.2 <https://docs.pgrouting.org/3.2/en/pgr_bipartite.html>`__)



Description
-------------------------------------------------------------------------------
A bipartite graph is a graph with two sets of vertices which are connected to each other, but not within themselves.
A bipartite graph is possible if the graph coloring is possible using two colors such that vertices in a set are colored with the same color.

**The main Characteristics are:**

- The algorithm works in undirected graph only.
- The returned values are not ordered.
- The algorithm checks graph is bipartite or not. If it is bipartite then it returns the node along with two colors `0` and `1` which represents two different sets.
- If graph is not bipartite then algorithm returns empty row.
- Running time: :math:`O(V + E)`

Signatures
-------------------------------------------------------------------------------


.. code-block:: sql

    .. pgr_bipartite(edges_sql)
    RETURNS SET OF (vid,color)


:Example: The pgr_bipartite algorithm with and edge_sql as a parameter when graph is bipartite:

.. literalinclude:: doc-bipartite.queries
   :start-after: --q1
   :end-before: --q2


.. index::
    single: bipartite (Single Vertex) - Experimental


Parameters
-------------------------------------------------------------------------------

============== ============ =================================================
Column         Type           Description
============== ============ =================================================
**edges_sql**   ``TEXT``    SQL query as described above.
============== ============ =================================================


Inner query
-------------------------------------------------------------------------------

:Edges SQL: an SQL query of an **undirected** graph, which should return
            a set of rows with the following columns:

.. include:: traversal-family.rst
   :start-after: edges_sql_start
   :end-before: edges_sql_end

Result Columns
-------------------------------------------------------------------------------

.. result columns start

Returns SET OF ``(vertex_id, color_id)``

===============  =========== ====================================================
Column           Type        Description
===============  =========== ====================================================
**vertex_id**    ``BIGINT``  Identifier of the vertex.
**color_id**     ``BIGINT``  Identifier of the color of the vertex.
                             `0`: `white`
                             `1`: `black`

===============  =========== ====================================================

Additional Examples
------------------------------------------------------------------------------------------

The examples in this section use the following :ref:`fig1`


:Example: It is possible that the cyclic graph can also be bipartite:

.. literalinclude:: doc-bipartite.queries
    :start-after: --q2
    :end-before: --q3


See Also
-------------------------------------------------------------------------------

* https://www.boost.org/doc/libs/1_50_0/libs/graph/doc/is_bipartite.html
* https://en.wikipedia.org/wiki/Bipartite_graph
* https://www.cs.cmu.edu/afs/cs/academic/class/15210-f11/www/lectures/16/lecture16.pdf
* :doc:`sampledata` network.

.. rubric:: Indices and tables

* :ref:`genindex`
* :ref:`search`