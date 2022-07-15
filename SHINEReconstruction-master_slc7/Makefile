#############################################################
###  User definitions section:

###  Generated input XML files to Shine
GENERATEDXMLS := $(patsubst %.xml.in,%.xml,$(wildcard *.xml.in))
###  The name of the raw input file.
INPUTFILE = $(DOCPATH)/SampleEvents/run-009917x054_exampleevent.raw
###  The global key from which we would like to start calibration
INPUTGLOBALKEY := 09_040


###  Symbolic targets (not real generated files)
.PHONY: tools clean

###  Generate necessary input XMLs and programs
tools: $(GENERATEDXMLS) 

#############################################################
###  The parts below are for experts only (steering the compilation):
###  set up input XML configuration and compile analyzer programs

SHINEOFFLINECONFIG := shine-offline-config
CONFIGFILES := $(shell $(SHINEOFFLINECONFIG) --config)
DBFILES := $(shell $(SHINEOFFLINECONFIG) --db-path)
DOCPATH := $(shell $(SHINEOFFLINECONFIG) --doc)

ifeq "$(strip $(CXX))" ""
  CXX := g++
endif

###  For generating input XMLs
%: %.in
	@echo -n "Generating $@ file..."
	@ sed -e 's!@''CONFIGDIR@!$(CONFIGFILES)!g;s!@''SHINEDBDIR@!$(DBFILES)!g' $< >$@
	@echo "done"
	@chmod +x runModuleSeq.sh

###  For compiling analyzer programs
%: %.C
	$(CXX) -o $@ $< `root-config --cflags` `root-config --libs`
%: %.cpp
	$(CXX) -o $@ $< `root-config --cflags` `root-config --libs`
%: %.cc
	$(CXX) -o $@ $< `root-config --cflags` `root-config --libs`
%: %.cxx
	$(CXX) -o $@ $< `root-config --cflags` `root-config --libs`

###  For cleaning up
clean:
	rm -f $(GENERATEDXMLS) *.root *.pdf produce merge calibrate run
