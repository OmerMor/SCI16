/* SCI selector values */


#ifndef	SELECTOR_H

#define	SELECTOR_H


enum	objOffsets {
	actX,
	actY,
	actSignal,
	actView,
	actLoop,
	actCel,
	actPri,
	actLS,
	actNS,
	actBR,
	actBRBottom,
	actUB,
	actXStep,
	actYStep,
	actLooper,
	actHeading,
	actMover,
	actMoveSpeed,
	actIllegalBits,
	motClient,
	motX,
	motY,
	motDX,
	motDY,
	motI1,
	motI2,
	motDI,
	motIncr,
	motXAxis,
	motMoveCnt,
	motXLast,
	motYLast,
	avClient,
	avHeading,
	jmpXStep,
	jmpYStep,
	evX,
	evY,
	evType,
	evMsg,
	evMod,
	evClaimed,
	sndBLANK1,
	sndBLANK2,
	sndBLANK3,
	sndBLANK4,
	sndBLANK5,
	actXLast,
	actYLast,
	actZ,
	syncTime,
	syncCue,
	sndBLANK6,
	sndBLANK7,
	sndBLANK8,
	OBJOFSSIZE
	};




#define	s_y					0
#define	s_x					1
#define	s_view				2
#define	s_loop				3
#define	s_cel					4
#define	s_underBits			5
#define	s_nowSeen			6
#define	s_nowSeenT			s_nowSeen
#define	s_nowSeenL			7
#define	s_nowSeenB			8
#define	s_nowSeenBottom	s_nowSeenB
#define	s_nowSeenR			9
#define	s_nsLeft				s_nowSeenL
#define	s_nsRight			s_nowSeenR
#define	s_nsTop				s_nowSeenT
#define	s_nsBottom			s_nowSeenB
#define	s_lastSeen			10
#define	s_lastSeenL			11
#define	s_lastSeenBottom 	12
#define	s_lastSeenR			13
#define	s_signal				14
#define	s_illegalBits		15
#define	s_baseRect			16
#define	s_baseRectT			s_baseRect
#define	s_baseRectL			17
#define	s_baseRectB			18
#define	s_baseRectR			19
#define	s_brLeft				s_baseRectL
#define	s_brRight			s_baseRectR
#define	s_brTop				s_baseRectT
#define	s_brBottom			s_baseRectB
#define	s_name				20
#define	s_key					21
#define	s_time				22
#define	s_text				23
#define	s_elements			24
#define	s_color				25
#define	s_back				26
#define	s_mode				27
#define	s_style				28
#define	s_state				29
#define	s_font				30
#define	s_type				31
#define	s_window				32
#define	s_cursor				33
#define	s_max					34
#define	s_mark				35
#define	s_who					36
#define	s_message			37
#define	s_edit				38
#define	s_play				39
#define	s_number				40
#define	s_nodePtr			41
#define	s_client 			42
#define	s_dx 					43
#define	s_dy 					44
#define	s_bMoveCnt 			45
#define	s_bI1 				46
#define	s_bI2 				47
#define	s_bDi 				48
#define	s_bXAxis 			49
#define	s_bIncr 				50
#define	s_xStep 				51
#define	s_yStep 				52
#define	s_moveSpeed			53
#define	s_cantBeHere 		54
#define	s_heading			55
#define	s_mover				56
#define	s_doit				57
#define	s_isBlocked			58
#define	s_looper				59
#define	s_priority			60
#define	s_modifiers			61
#define	s_replay				62
#define	s_setPri				63
#define	s_at					64
#define	s_next				65
#define	s_done				66
#define	s_width				67
#define	s_wordFail			68
#define	s_syntaxFail		69
#define	s_semanticFail		70
#define	s_pragmaFail		71
#define	s_said				72
#define	s_claimed			73
#define	s_value				74
#define	s_save				75
#define	s_restore			76
#define	s_title				77
#define	s_button				78
#define	s_icon				79
#define	s_draw				80
#define	s_delete				81
#define	s_z					82
#define	s_parseLang			83
#define	s_printLang			84
#define	s_subtitleLang		85
#define	s_size				86
#define	s_points				87
#define	s_palette			88
#define	s_dataInc			89
#define	s_handle				90
#define	s_min					91
#define	s_sec					92
#define	s_frame				93
#define	s_vol					94
#define	s_BLANK				95
#define	s_perform			96
#define	s_moveDone			97
#define  s_topString			98
#define  s_flags				99
#define  s_quitGame			100
#define  s_restart			101
#define  s_hide				102
#define	s_vm_signal			103 
#define	s_scaleX				104
#define	s_scaleY				105
#define	s_maxScale			106
#define	s_vanishingX		107
#define	s_vanishingY		108 

#endif
