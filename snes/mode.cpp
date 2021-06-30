#include "mode.hpp"

std::array<LayerPriorityTable, 8> prios_for_mode =
    {{// Mode 0
      {
          {3, 0},
          {2, 0},
          {Layers::OBJ, 0},
          {3, 1},
          {2, 1},
          {Layers::OBJ, 1},
          {1, 0},
          {0, 0},
          {Layers::OBJ, 2},
          {1, 1},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 1
      {
          {2, 0},
          {Layers::OBJ, 0},
          {Layers::OBJ, 1},
          {1, 0},
          {0, 0},
          {Layers::OBJ, 2},
          {1, 1},
          {0, 1},
          {Layers::OBJ, 3},
          {2, 1}},
      // Mode 2
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 3
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 4
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 5
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 6
      {
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 7
      {
          { 0, 0 }
      }}};