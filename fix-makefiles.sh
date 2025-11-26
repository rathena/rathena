#!/bin/bash
# Post-configure Makefile fixes for build dependencies

echo "Applying Makefile patches..."

# Fix src/common/Makefile - add p2p_coordinator.o and quic.o
sed -i 's/conf.o msg_conf.o cli.o sql.o database.o$/conf.o msg_conf.o cli.o sql.o database.o p2p_coordinator.o quic.o/' src/common/Makefile

# Fix src/login/Makefile - add libcurl
sed -i 's/-lresolv -lm$/-lresolv -lm -lcurl/' src/login/Makefile

# Fix src/char/Makefile - add libcurl
sed -i 's/-lresolv -lm$/-lresolv -lm -lcurl/' src/char/Makefile

# Fix src/map/Makefile - add Python includes and libs
# First, add Python variables after HTTPLIB section
sed -i '/^HTTPLIB_INCLUDE = /a\\nPYTHON_INCLUDE = $(shell python3-config --includes)\nPYTHON_LIBS = $(shell python3-config --libs --embed 2>/dev/null || python3-config --libs)' src/map/Makefile

# Add Python libs to map-server link command
sed -i 's/-lresolv -lm$/-lresolv -lm $(PYTHON_LIBS) -lcurl/' src/map/Makefile

# Add Python includes to compilation rules
sed -i 's/\$(RAPIDYAML_INCLUDE) -I\/usr\/include\/mysql/$(RAPIDYAML_INCLUDE) $(PYTHON_INCLUDE) -I\/usr\/include\/mysql/g' src/map/Makefile

echo "Makefile patches applied successfully"