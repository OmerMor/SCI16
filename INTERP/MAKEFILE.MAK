######################################################################
#
# makefile
#
# Makefile for the SCI interpreter and assorted drivers and utilities.
#
#
######################################################################

#
# Files to be compiled for debugging.  All others will be optimized.
#
debug =
#
# Files to be compiled with a listing.
#
listing = 


############ DEFAULT TARGETS ############
# sierra   no debug
# sierram  no debug, menu bars enabled
# sci      debug
# scitestr debug, menu bars enabled

prod: sierra sierram sci scitestr

# Do not use builtins (default) dependencies or actions -- everything
# is explicitly declared here.
.SUFFIXES

# Execute commands directly, without invoking command.com.  
.NOSHELL

# Version control stuff will be done unless we set dovcs=FALSE on the
# make command line.
dovcs = TRUE

.INIT:
	#
	# Do not use any Microsoft files for the interpreter
	#
	@%setenv include=.;i:\interp\sci\ibm
	@%setenv lib=.;i:\interp\sci\ibm
	#
	@%set masmFlags = -Fm -c -Cp -nologo -W2
	# -Fm		create a linker map file
	# -c		assemble-only (don't link)
	# -Cp		preserve case of all identifiers
	# -nologo	supress NMAKE copyright message
	# -W2		don't warn about extended jumps
	#
	# make sure our local clock agrees with network
	#
	@systime


.DEINIT:
	#
	# If we successfully made the executables, increment the version number
	# and put a version number label in the vcs files.
	#
	%if !%status && $(errors) == FALSE && $(makeExe) == TRUE
		#
		# If there is a version file, do version control stuff.
		#
		%if %file($(verFile))
			#
			# If there is a VCS subdirectory, mark all files in it with
			# the current version number.
			#
			%if %exists(vcs) && !%file(vcs) && $(dovcs) == TRUE
				@%echo *** Stamping version control files with current version number.
				@+oneline vcs -vv@$(verFile)@ vcs\\*.* >___dovcs.bat
				@+___dovcs
			%endif
			@inc_it $(verFile)
		%endif
	%endif



#
# Files to be compiled for speed optimization.  All others will be 
# optimized for space.
#
speed  = string.c list.c animate.c motion.c getpath.c resource.c altres.c

#
# Files which are used to make utilities.  These will use the default
# libraries.
#
utils = 


#
# The source files which do not vary between interpreters.
#
s1	= stdio.c stdioasm.s scifgets.c string.c stdlib.c ctype.c window.c
s2	= io.s config.c event.c script.c list.c altres.c animate.c armasm.s
s3	= math.s savevars.s restart.c sound.c motion.c flist.c
s4	= savegame.c getpath.c midi.s fardata.c message.c trig.c audio.c
s5	= aud.s sync.c errmsg.c platform.c info.c scale.c mono.s
s6	= ems.s  eventasm.s extmem.s  dialog.c  fileio.s  intrpt.c  intasm.s language.c mouse.c xms.s mouseasm.s
s7	= picture.c graph.s  graph256.s cels.s resname.c palette.c pk.c movie.c
s8	= object.c start.c startasm.s

src	= $(s1) $(s2) $(s3) $(s4) $(s5) $(s6) $(s7) $(s8)
base	= $[f,,$(src),o]

#
# Debugging files 
#
nodbg	= debug.o pmachine.o pmachasm.o objasm.o dos.o debugasm.o \
	resource.o sci.o opcodes.o kerndisp.o kernel.o text.o volload.o
dbg	= debug.od pmachine.od pmachasm.od objasm.od dos.od debugasm.od  \
	resource.od sci.od opcodes.od kerndisp.od kernel.od text.od volload.od

#
# Tester files
#
notst	= menu.o
tst	= menu.ot

#
# This file must come last for the memory manager to work
#
lastNodbg	= memmgr.o
lastDbg		= memmgr.od


#
# User (debugless-vols) interpreter 
#
Objs	 = $(base) $(nodbg) $(notst) $(lastNodbg)
tObjs	 = $(base) $(nodbg) $(tst)   $(lastNodbg)

#
# Inhouse debug (file, tester and vols) interpreters
#
dObjs	 = $(base) $(dbg)   $(notst) $(lastDbg)
dtObjs = $(base) $(dbg)   $(tst)   $(lastDbg)

############ DEBUG-NODEBUG RESOURCES  ############

sciBaseSetup:
# let all source files know that we are compiling the interpreter
	@%set cDefs = -DSCI
	@%set mDefs = -DSCI -DIBM

sciNoDbgSetup:
	@%do sciBaseSetup

sciNoDbgTSetup:
	@%do sciNoDbgSetup
	@%set cDefs = $(cDefs) -DTESTER
	@%set mDefs = $(mDefs) -DTESTER

sciDbgSetup:
	@%do sciBaseSetup
	@%set cDefs = $(cDefs) -DDEBUG
	@%set mDefs = $(mDefs) -DDEBUG

sciDbgTSetup:
	@%do sciDbgSetup
	@%set cDefs = $(cDefs) -DTESTER
	@%set mDefs = $(mDefs) -DTESTER

############ BUILD INTERPRETERS ##########

sierra:   sierraSetup sierra.exe

sierram:  sierramSetup sierram.exe

sci:      sciSetup sci.exe

scitestr: scitestrSetup scitestr.exe

sierraSetup:
	@%do sciNoDbgSetup

sierramSetup:
	@%do sciNoDbgTSetup

sciSetup:
	@%do sciDbgSetup

scitestrSetup:
	@%do sciDbgTSetup

sierra.exe: $(Objs)
	@%set verFile = sci.ver
	@%do Link 

sierram.exe: $(tObjs)
	@%set verFile = sci.ver
	@%do Link 

sci.exe: $(dObjs)
	@%set verFile = sci.ver
	@%do Link 

scitestr.exe: $(dtObjs)
	@%set verFile = sci.ver
	@%do Link 


############ METHODS FOR BUILDING OBJECT FILES  ############

### We only do medium model assembly right now ###

.DEFAULT.s:		#don't worry about trying to make .s files

debugasm.od:	[debugasm.s]
	@%do	Assemble

memmgr.od:	[memmgr.s]
	@%do	Assemble

objasm.od:	[objasm.s]
	@%do	Assemble

opcodes.od:	[opcodes.s]
	@%do	Assemble

kerndisp.od:	[kerndisp.s]
	@%do	Assemble

pmachasm.od:	[pmachasm.s]
	@%do	Assemble

###### ASSEMBLY: conditionals and general rules######

.s.o:
	@%do	Assemble

Assemble:
	@%set	srcFile = $<
	@%set	objFile = $@
#
	%if %member($<, $(debug))
		@%set masmFlags = $(masmFlags) -Zd -Zi
		#	-Zd	put line number info in object file
		#	-Zi	put CodeView info in object file
	%endif
#
	%if %member($<, $(listing))
		@%set masmFlags = $(masmFlags) -Sg
		#	-Sg	turn on listing of assembly code
	%endif
#
	-:ml $(mDefs) $(masmFlags) -Fo$(objFile) -Ta$(srcFile)
	%if %status
		@%set errors = TRUE
		@%exit 1
	%endif
	



###### C COMPILATION: conditionals and general rules######

.DEFAULT.c:		#don't worry about trying to make .c files

debug.od:	[debug.c]
	%do Compile

dos.od:	[dos.c]
	%do Compile

resource.od:	[resource.c]
	%do Compile

sci.od:	[sci.c]
	%do Compile

kernel.od:	[kernel.c]
	%do Compile

text.od:	[text.c]
	%do Compile

volload.od:	[volload.c]
	%do Compile

pmachine.od:	[pmachine.c]
	%do Compile

menu.ot:	[menu.c]
	%do Compile

.c.o:
	%do Compile


Compile:
#
#	Set any debugging-related options
#
	@%set	srcFile = $<
	@%set	objFile = $@
	@%set cFlags = /Fm -c -DLINT_ARGS -DIBM -W3 -Gs -Zp -J -AM
	# -c		compile only
	# -DLINT_ARGS	define LINT_ARGS, to enable argument checking
	# -DIBM		define IBM
	# -W3		give maximum number of error messages
	# -Gs		disable stack checking
	# -Zp		pack all structures
	# -J		chars are unsigned by default
	# -AM		medium model (large code, small data)
	#
	%if %member($<, $(debug))
		@%set cFlags = $(cFlags) -Zi -Od
		# -Zi	put CodeView info in object file
		# -Od	disable optimizations
	%elseif %member($<, $(speed))
		@%set cFlags = $(cFlags) -Oalt
	%else
		@%set cFlags = $(cFlags) -Os
	%endif
#
	%if %member($<, $(listing))
		@%set cFlags = $(cFlags) -Fc
	%endif
#
	%if !%member($<, $(utils))
		@%set cFlags = $(cFlags) -Zl
		# -Zl	generate references to standard libraries in object file
	%endif
#
	-:cl $(cDefs) $(cFlags) -Fo$(objFile) $(srcFile)
	%if %status
		@%set errors = TRUE
		@%exit 1
	%endif
	


################### LINKING EXECUTABLES ######################
#
#	As a means of tracking who built an executable and when and
#	where it was built, we generate the file 'info.c' when we're
#	about to build the first executable.  Once that it is generated,
#	we re-invoke make with the 'makeExe' flag set to true.  This
#	then builds all executables with the new information in them.

makeExe = FALSE
errors = FALSE

lFlags = /NOI /MAP /NOE /CO
# Linker flags
# /NOI		case sensitive
# /MAP		create a link map
# /NOE		no external dictionary (whatever that means)
# /CO 		generate codeview information

Link:    
	%if $(makeExe) == FALSE
		#
		# This is the primary invocation of make and we're about
		# to build an executable file.  Generate the information
		# file and re-invoke make to build all executables.
		#
		@%do BuildInfo
		&@:make makeExe=TRUE $(M_ARGS)
	%else
		#
		# This is the recursive invocation of make to build all
		# executables.  Just do a normal link.
		#
		-:link $(lFlags) <@<
$[s," +\n",$**]
$*.exe
$*.map
libh.lib implode.lib;
<
		%if %status
			@%set errors = TRUE
			@%exit 1
		%endif
	%endif


	
############ SUBROUTINES ############

info.c:
	@%do	BuildInfo

BuildInfo:
	#
	# Put the following information into a C file to be compiled and
	# linked into the interpreters being made with this invocation of make:
	#
	#	Current date and time
	#	The VCSID of the person doing the make
	#	The pathname of the directory in which the make is taking place
	#	The current version number (which will be auto-incremented if
	#		the make succeeds).
	#	The text from the file comment/comment, if it exists.
	#
	@+echo char makeDate[] = __DATE__; 								>info.c
	@+echo char makeTime[] = __TIME__;								>>info.c
	@+echo char maker[] = "$(VCSID)";								>>info.c
	@+echo char makeLocation[] = "$[t,\,/,$[l,$(CWD)]]";		>>info.c
	#
	%if %file($(verFile))
		@+oneline char version[] = "@sci.ver@";					>>info.c
	%else
		@+echo char version[] = "No version number";				>>info.c
	%endif
	#
	%if %exists(comment\comment)
		@+oneline char makeComment[] = "@comment\comment@";	>>info.c
	%else
		@+echo char makeComment[] = "Not a release version.";	>>info.c
	%endif
