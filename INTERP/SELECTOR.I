;-------------------------------------------------------------------
;	selector.i
;
;	SCI selector values
;-------------------------------------------------------------------

ifndef	SELECTOR_I

SELECTOR_I	equ	1


s_y					equ	0
s_x					equ	1
s_view				equ	2
s_loop				equ	3
s_cel					equ	4
s_underBits			equ	5
s_nowSeen			equ	6
s_nowSeenT			equ	s_nowSeen
s_nowSeenL			equ	7
s_nowSeenB			equ	8
s_nowSeenBottom	equ	s_nowSeenB
s_nowSeenR			equ	9
s_nsLeft				equ	s_nowSeenL
s_nsRight			equ	s_nowSeenR
s_nsTop				equ	s_nowSeenT
s_nsBottom			equ	s_nowSeenB
s_lastSeen			equ	10
s_lastSeenL			equ	11
s_lastSeenBottom 	equ	12
s_lastSeenR			equ	13
s_signal				equ	14
s_illegalBits		equ	15
s_baseRect			equ	16
s_baseRectT			equ	s_baseRect
s_baseRectL			equ	17
s_baseRectB			equ	18
s_baseRectR			equ	19
s_brLeft				equ	s_baseRectL
s_brRight			equ	s_baseRectR
s_brTop				equ	s_baseRectT
s_brBottom			equ	s_baseRectB
s_name				equ	20
s_key					equ	21
s_time				equ	22
s_text				equ	23
s_elements			equ	24
s_color				equ	25
s_back				equ	26
s_mode				equ	27
s_style				equ	28
s_state				equ	29
s_font				equ	30
s_type				equ	31
s_window				equ	32
s_cursor				equ	33
s_max					equ	34
s_mark				equ	35
s_who					equ	36
s_message			equ	37
s_edit				equ	38
s_play				equ	39
s_number				equ	40
s_nodePtr			equ	41
s_client 			equ	42
s_dx 					equ	43
s_dy 					equ	44
s_bMoveCnt 			equ	45
s_bI1 				equ	46
s_bI2 				equ	47
s_bDi 				equ	48
s_bXAxis 			equ	49
s_bIncr 				equ	50
s_xStep 				equ	51
s_yStep 				equ	52
s_moveSpeed			equ	53
s_cantBeHere 		equ	54
s_heading			equ	55
s_mover				equ	56
s_doit				equ	57
s_isBlocked			equ	58
s_looper				equ	59
s_priority			equ	60
s_modifiers			equ	61
s_replay				equ	62
s_setPri				equ	63
s_at					equ	64
s_next				equ	65
s_done				equ	66
s_width				equ	67
s_wordFail			equ	68
s_syntaxFail		equ	69
s_semanticFail		equ	70
s_pragmaFail		equ	71
s_said				equ	72
s_claimed			equ	73
s_value				equ	74
s_save				equ	75
s_restore			equ	76
s_title				equ	77
s_button				equ	78
s_icon				equ	79
s_draw				equ	80
s_delete				equ	81
s_z					equ	82
s_parseLang			equ	83
s_printLang			equ	84
s_subtitleLang		equ	85
s_size				equ	86
s_points				equ	87
s_palette			equ	88
s_dataInc			equ	89
s_handle				equ	90
s_min					equ	91
s_sec					equ	92
s_frame				equ	93
s_vol					equ	94
s_BLANK				equ	95
s_perform			equ	96
s_moveDone			equ	97
s_topString			equ	98
s_flags				equ	99
s_quitGame			equ	100
s_restart			equ	101
s_hide				equ	102


endif

