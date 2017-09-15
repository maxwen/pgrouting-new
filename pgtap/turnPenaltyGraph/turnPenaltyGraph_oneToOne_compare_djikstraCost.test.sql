/*PGR-GNU*****************************************************************

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

********************************************************************PGR-GNU*/

\i setup.sql

SELECT plan(144);

UPDATE edge_table SET cost = cost*-1 WHERE cost < 1; 

DROP TABLE IF EXISTS turn_penalty_table;
CREATE TABLE turn_penalty_table(
  seq integer,
  source bigint,
  target bigint,
  cost float,
  original_source_vertex bigint,
  original_source_edge bigint,
  original_target_vertex bigint,
  original_target_edge bigint);

CREATE or REPLACE FUNCTION turnPenaltyGraphCompareDijkstra(cant INTEGER default 17)
RETURNS SETOF TEXT AS
$BODY$
DECLARE
inner_sql TEXT;
dijkstra_sql TEXT;
turn_penalty_graph_sql TEXT;
source_arr TEXT;
incoming TEXT;
outgoing TEXT;
target_arr TEXT;
BEGIN

    inner_sql := 'select id, source, target, cost from edge_table';
    EXECUTE 'insert into turn_penalty_table select * from pgr_turnPenaltyGraph($$' || inner_sql || '$$)';

    FOR i IN 1.. cant BY 2 LOOP
        FOR j IN 1.. cant LOOP

            -- If i = j there is no shortest path
            CONTINUE WHEN i = j;

            source_arr := array_to_string(array(select source from turn_penalty_table where original_source_vertex = i), ',');
            -- If the source_arr is empty that means there are no outgoing edges from vertex i in the original graph
            IF source_arr = '' THEN
              inner_sql := 'SELECT source FROM edge_table WHERE source = ' || i;
              outgoing := 'SELECT source FROM turn_penalty_table WHERE original_source_vertex = ' || i;
              RETURN query SELECT set_eq(outgoing, inner_sql, outgoing);
              CONTINUE WHEN TRUE;
            END IF;

            target_arr := array_to_string(array(select target from turn_penalty_table where original_target_vertex = j),',');
            -- If the target_arr is empty that means there are no incoming edges to vertex j in the original graph
            IF target_arr = '' THEN
              inner_sql := 'SELECT target FROM edge_table WHERE target = ' || j;
              incoming := 'SELECT target FROM turn_penalty_table WHERE original_target_vertex = ' || j;
              RETURN query SELECT set_eq(incoming, inner_sql, incoming);
              CONTINUE WHEN TRUE;
            END IF;

            inner_sql := 'SELECT seq as id, source, target, cost FROM turn_penalty_table';

            turn_penalty_graph_sql := 'SELECT agg_cost FROM pgr_dijkstraCost($$' || inner_sql || '$$, ARRAY[' 
                || source_arr || '], ARRAY[' 
                || target_arr
                || ']) ORDER BY agg_cost DESC LIMIT 1';

            inner_sql := 'SELECT id, source, target, cost FROM edge_table';

            dijkstra_sql := 'SELECT agg_cost FROM pgr_dijkstraCost($$' || inner_sql || '$$, ' || i || ', ' || j
                || ')';

            RETURN query SELECT set_eq(turn_penalty_graph_sql, dijkstra_sql, turn_penalty_graph_sql);

        END LOOP;
    END LOOP;

    RETURN;
END
$BODY$
language plpgsql;

SELECT * from turnPenaltyGraphCompareDijkstra();


SELECT * FROM finish();
ROLLBACK;

