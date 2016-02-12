;	window.i

ifndef WINDOW_I
WINDOW_I equ 1

ifndef GRAPH_I
include graph.i
endif

ifndef LIST_I
include list.i
endif

externdef wmgrPort: RGrafPortPtr
externdef windowList: List

endif
