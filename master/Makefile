PROJECT = RfMeshMaster

LIBS = RfMeshThreading RfMeshTransceiverBase rfm70 RfMeshTransceiverRfm70 RfMeshAppMessageQueue RfMeshMeshMessageBacklog RfMeshMeshMessageTracker RfMeshRoutingTable RfMeshDatabase RfMeshNode RfMeshThreadNode RfMeshThreadTime RfMeshThreadSocket RfMeshThreadDataLogger
LIBSH = pins rfm70reg RfMeshLogger RfMeshObserver RfMeshProtocol RfMeshUtils RfMeshNodeType RfMeshNodeConfig

PWD = $(shell pwd)
SOURCEPATH = source
BUILDPATH = build
OUTPATH = out
BINPATH = 
LIBPATH = ../lib
WEBPATH = ../web
LOGPATH = ../log

CFLAGS	= -c -O3 -w -I/usr/local/include -I/usr/include/mongo -I$(SOURCEPATH)
LDFLAGS	= -L/usr/local/lib $(LDLIBS) -llog4cxx -lwiringPi -lsqlite3 -lxerces-c -ljsoncpp -lmongoclient -lboost_thread-mt -lboost_filesystem -lboost_program_options -lboost_system -L$(BUILDPATH) -lm

all: clean source out run

noclean: cleandata source out run	
	
clean:
	-sudo killall $(PROJECT)
	-sudo killall node
	rm -rf $(SOURCEPATH)
	rm -rf $(BUILDPATH)
	rm -rf $(OUTPATH)
	rm -rf $(LOGPATH)

cleandata:
	-sudo killall $(PROJECT)
	-sudo killall node
#	rm -f $(LOGPATH)/*.log
	
source: $(SOURCEPATH)/$(PROJECT).h $(SOURCEPATH)/$(PROJECT).cpp $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBS))) $(addprefix $(SOURCEPATH)/, $(addsuffix .cpp, $(LIBS))) $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBSH)))

$(SOURCEPATH)/%.h: %.h
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.cpp: %.cpp
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.h: $(LIBPATH)/%.h
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.cpp: $(LIBPATH)/%.cpp
	mkdir -p $(SOURCEPATH)
	cp $< $@
	
$(BUILDPATH)/lib/%.o: $(SOURCEPATH)/%.cpp
	mkdir -p $(BUILDPATH)/lib
	$(BINPATH)g++ $(CFLAGS) -c $< -o $@

$(BUILDPATH)/%.o: $(SOURCEPATH)/%.cpp
	mkdir -p $(BUILDPATH)
	$(BINPATH)g++ $(CFLAGS) -c $< -o $@

$(OUTPATH)/$(PROJECT): $(BUILDPATH)/$(PROJECT).o $(addprefix $(BUILDPATH)/lib/, $(addsuffix .o, $(LIBS)))
	mkdir -p $(OUTPATH)
	$(BINPATH)g++ -o $@ $(BUILDPATH)/$(PROJECT).o $(addprefix $(BUILDPATH)/lib/, $(addsuffix .o, $(LIBS))) $(LDFLAGS)

out: $(OUTPATH)/$(PROJECT)
	cp -f RfMeshConfig.xml $(OUTPATH)/
	
run:
	-sudo service mongodb stop
	-sudo rm /var/lib/mongodb/mongod.lock
	-sudo service mongodb start
	mkdir -p $(LOGPATH)
	cd $(OUTPATH); nohup sudo ./$(PROJECT) >> ../$(LOGPATH)/$(PROJECT).log 2>&1 &
	cd $(WEBPATH); nohup node RfMeshWeb.js >> $(LOGPATH)/RfMeshWeb.log 2>&1 &
	tail -f $(LOGPATH)/$(PROJECT).log

