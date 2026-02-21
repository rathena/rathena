# rAthena Docker Makefile
# Provides convenient commands for common Docker operations
# All commands should be run from the tools/docker/ directory

.PHONY: help start init stop nuke logs build build-login build-char build-map \
        run run-login run-char run-map admin account shell ps clean configure rebuild

# Default target
help:
	@echo "rAthena Docker Helper Commands"
	@echo "=============================="
	@echo ""
	@echo "Infrastructure:"
	@echo "  make start       - Start all containers (equivalent to 'docker compose up -d')"
	@echo "  make init        - Start database and builder containers only"
	@echo "  make stop        - Stop all containers and remove them"
	@echo "  make nuke        - Stop all containers and delete database volume"
	@echo "  make ps          - Show container status"
	@echo "  make shell       - Open a shell in the builder container"
	@echo ""
	@echo "Building (compiles and restarts services automatically):"
	@echo "  make build       - Compile all servers and restart game services"
	@echo "  make build-login - Compile and restart login-server"
	@echo "  make build-char  - Compile and restart char-server"
	@echo "  make build-map   - Compile and restart map-server"
	@echo "  make clean       - Run make clean"
	@echo ""
	@echo "Running Game Servers (containers):"
	@echo "  make run         - Start all game servers (login, char, map)"
	@echo "  make run-login   - Start login-server container"
	@echo "  make run-char    - Start char-server container"
	@echo "  make run-map     - Start map-server container"
	@echo ""
	@echo "Account Management:"
	@echo "  make admin [NAME=name] [PASS=pass] [SEX=M] [EMAIL=email] - Create admin"
	@echo "  make account NAME=name PASS=pass SEX=M|F|S EMAIL=email [GROUP=0] - Create account"
	@echo ""
	@echo "Quick Start:"
	@echo "  make init && make build && make admin"
	@echo ""
	@echo "Other Commands:"
	@echo "  make logs        - Attach to container logs (Ctrl+C to exit)"

# ============================================================================
# Infrastructure Commands
# ============================================================================

# Start all containers
start:
	docker compose up -d

# Start only db and builder (for development/building)
init:
	docker compose up -d db builder

# Stop and remove containers
stop:
	docker compose down

# Stop and remove containers + delete database volume
nuke:
	docker compose down --volumes

# Attach to container logs (Ctrl+C to exit)
logs:
	docker compose logs -f

# Show container status
ps:
	docker compose ps

# Open shell in builder container
shell:
	docker compose run --rm builder bash

# ============================================================================
# Build Commands (run through builder container)
# ============================================================================

# Compile all servers and restart all game services
build:
	docker compose run --rm builder make server
	docker compose up -d --no-deps login char map

# Compile login-server and restart the service
build-login:
	docker compose run --rm builder make login-server
	docker compose up -d --no-deps login

# Compile char-server and restart the service
build-char:
	docker compose run --rm builder make char-server
	docker compose up -d --no-deps char

# Compile map-server and restart the service
build-map:
	docker compose run --rm builder make map-server
	docker compose up -d --no-deps map

# Run make clean
clean:
	docker compose run --rm builder make clean

# Configure the project (run ./configure)
configure:
	docker compose run --rm builder ./configure

# Full rebuild: clean + configure + build
rebuild: clean configure build

# ============================================================================
# Game Server Containers
# ============================================================================

# Start all game servers
run: run-login run-char run-map

# Start login-server container
run-login:
	docker compose up -d login

# Start char-server container
run-char:
	docker compose up -d char

# Start map-server container
run-map:
	docker compose up -d map

# ============================================================================
# Account Management
# ============================================================================

# Create an admin account (defaults: admin/admin, group 99)
# Usage: make admin [NAME=username] [PASS=password] [SEX=M|F|S] [EMAIL=email]
admin: NAME ?= admin
admin: PASS ?= admin
admin: SEX ?= M
admin: EMAIL ?= admin@rathena.dev
admin:
	@echo "Creating admin account..."
	@docker compose exec db mariadb -uragnarok -pragnarok ragnarok -e \
		"INSERT INTO login (account_id, userid, user_pass, sex, email, group_id, birthdate) \
		VALUES (2000000, '$(NAME)', MD5('$(PASS)'), '$(SEX)', '$(EMAIL)', 99, CURDATE()) \
		ON DUPLICATE KEY UPDATE userid='$(NAME)', user_pass=MD5('$(PASS)'), sex='$(SEX)', email='$(EMAIL)', group_id=99, birthdate=CURDATE();"
	@echo "Admin account created/updated (ID: 2000000):"
	@echo "  Username: $(NAME)"
	@echo "  Password: $(PASS)"
	@echo "  Sex: $(SEX)"
	@echo "  Email: $(EMAIL)"
	@echo "  Group ID: 99"

# Create a regular account (NAME, PASS, SEX, and EMAIL required)
# Usage: make account NAME=username PASS=password SEX=M|F|S EMAIL=email [GROUP=0]
account:
	@if [ -z "$(NAME)" ] || [ -z "$(PASS)" ] || [ -z "$(SEX)" ] || [ -z "$(EMAIL)" ]; then \
		echo "Error: NAME, PASS, SEX, and EMAIL are required"; \
		echo "Usage: make account NAME=myuser PASS=mypass SEX=M EMAIL=user@example.com [GROUP=0]"; \
		exit 1; \
	fi
	@echo "Creating account..."
	@docker compose exec db mariadb -uragnarok -pragnarok ragnarok -e \
		"INSERT INTO login (userid, user_pass, sex, email, group_id, birthdate) \
		VALUES ('$(NAME)', MD5('$(PASS)'), '$(SEX)', '$(EMAIL)', $(or $(GROUP),0), CURDATE()) \
		ON DUPLICATE KEY UPDATE user_pass=MD5('$(PASS)'), sex='$(SEX)', email='$(EMAIL)', group_id=$(or $(GROUP),0), birthdate=CURDATE();"
	@echo "Account created/updated:"
	@echo "  Username: $(NAME)"
	@echo "  Password: $(PASS)"
	@echo "  Sex: $(SEX)"
	@echo "  Email: $(EMAIL)"
	@echo "  Group ID: $(or $(GROUP),0)"
	