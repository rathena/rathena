#!/bin/bash
# Generate self-signed SSL certificates for development/testing
# For production, use Let's Encrypt or your certificate authority

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CERTS_DIR="$PROJECT_ROOT/certs"

echo "=================================="
echo "SSL Certificate Generation"
echo "=================================="
echo ""

# Create certs directory if it doesn't exist
mkdir -p "$CERTS_DIR"

# Certificate parameters
CERT_DAYS=365
CERT_COUNTRY="US"
CERT_STATE="State"
CERT_CITY="City"
CERT_ORG="rathena-AI-world"
CERT_UNIT="Development"
CERT_CN="localhost"
CERT_EMAIL="admin@localhost"

echo "Generating self-signed SSL certificate..."
echo "  Valid for: $CERT_DAYS days"
echo "  Common Name: $CERT_CN"
echo "  Output directory: $CERTS_DIR"
echo ""

# Generate private key and certificate
openssl req -x509 -newkey rsa:4096 -nodes \
  -keyout "$CERTS_DIR/server.key" \
  -out "$CERTS_DIR/server.crt" \
  -days $CERT_DAYS \
  -subj "/C=$CERT_COUNTRY/ST=$CERT_STATE/L=$CERT_CITY/O=$CERT_ORG/OU=$CERT_UNIT/CN=$CERT_CN/emailAddress=$CERT_EMAIL" \
  2>/dev/null

# Set appropriate permissions
chmod 600 "$CERTS_DIR/server.key"
chmod 644 "$CERTS_DIR/server.crt"

echo "✓ SSL certificate generated successfully!"
echo ""
echo "Files created:"
echo "  Private key: $CERTS_DIR/server.key (permissions: 600)"
echo "  Certificate: $CERTS_DIR/server.crt (permissions: 644)"
echo ""
echo "Certificate details:"
openssl x509 -in "$CERTS_DIR/server.crt" -noout -subject -dates
echo ""
echo "⚠️  WARNING: This is a SELF-SIGNED certificate for development only!"
echo "   For production, use Let's Encrypt or a trusted Certificate Authority."
echo ""
echo "To use Let's Encrypt in production:"
echo "  1. Install certbot: sudo apt-get install certbot"
echo "  2. Generate cert: sudo certbot certonly --standalone -d yourdomain.com"
echo "  3. Copy certs: sudo cp /etc/letsencrypt/live/yourdomain.com/{fullchain.pem,privkey.pem} $CERTS_DIR/"
echo "  4. Update .env: SSL_CERTFILE=$CERTS_DIR/fullchain.pem"
echo "                  SSL_KEYFILE=$CERTS_DIR/privkey.pem"
echo ""
echo "=================================="
echo "SSL Setup Complete!"
echo "=================================="