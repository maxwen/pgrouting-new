/*PGR-GNU*****************************************************************
File: _trspVia.sql

Function's developer:
Copyright (c) 2022 Celia Virginia Vergara Castillo

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

------------------
-- pgr_trspVia
------------------

--v3.0
CREATE FUNCTION _pgr_trspVia(
  TEXT, -- edges
  TEXT, -- restrictions
  ANYARRAY, -- via
  BOOLEAN,  -- directed
  BOOLEAN,  -- strict
  BOOLEAN,  -- U_turn_on_edge

  OUT seq INTEGER,
  OUT path_id INTEGER,
  OUT path_seq INTEGER,
  OUT start_vid BIGINT,
  OUT end_vid BIGINT,
  OUT node BIGINT,
  OUT edge BIGINT,
  OUT cost FLOAT,
  OUT agg_cost FLOAT,
  OUT route_agg_cost FLOAT)
RETURNS SETOF RECORD AS
'MODULE_PATHNAME'
LANGUAGE c VOLATILE STRICT;

-- COMMENTS

COMMENT ON FUNCTION _pgr_trspVia(TEXT, TEXT, ANYARRAY, BOOLEAN, BOOLEAN, BOOLEAN)
IS 'pgRouting internal function';
