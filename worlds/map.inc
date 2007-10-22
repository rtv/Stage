# map.inc - useful setup for a floorplan bitmap
# Authors: Richard Vaughan
# $id$

define floorplan model
(
  # sombre, sensible, artistic
  color "gray30"

  # most maps will need a bounding box
  #boundary 1

  gui_nose 0
  gui_grid 1
  gui_movemask 0

  # maps can not be picked up and moved 
  gripper_return 0
  laser_return 1
)
