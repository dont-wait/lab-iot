#!/usr/bin/env bash
# Smoke test tren server that (Node-RED + Express).
#
# Chay server truoc:
#   PORT=1881 npm start
# Sau do:
#   BASE_URL=http://127.0.0.1:1881 bash tests/smoke_real_server.sh
#
# Ket qua duoc ghi vao evidence/real_server.log.

set -u

BASE_URL="${BASE_URL:-http://127.0.0.1:1881}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/evidence/real_server.log"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

VIEWER_JAR="$TMP/viewer.jar"
ADMIN_JAR="$TMP/admin.jar"
PASS=0
FAIL=0

log() { printf '%s\n' "$*" | tee -a "$OUT"; }

expect() {
    local name="$1" expected="$2" actual="$3"
    if [ "$expected" = "$actual" ]; then
        PASS=$((PASS + 1))
        log "PASS  $name — $actual"
    else
        FAIL=$((FAIL + 1))
        log "FAIL  $name — expected=$expected actual=$actual"
    fi
}

: > "$OUT"
log "=========================================================================="
log "SMOKE TEST SERVER THAT - LAB 7 BAI 3"
log "Thoi diem: $(date -Iseconds)"
log "Base URL: $BASE_URL"
log "=========================================================================="

log ""
log "[1] Chua dang nhap"
ANON_UI="$(curl --max-time 10 -s -o /dev/null -w '%{http_code}' "$BASE_URL/ui")"
expect "anon /ui -> 302" "302" "$ANON_UI"
ANON_API="$(curl --max-time 10 -s -o /dev/null -w '%{http_code}' "$BASE_URL/api/state")"
expect "anon /api/state -> 401" "401" "$ANON_API"

log ""
log "[2] Tai khoan viewer (read-only)"
VIEWER_LOGIN="$(curl --max-time 10 -s -c "$VIEWER_JAR" \
    -H 'Content-Type: application/json' -H "Origin: $BASE_URL" \
    -d '{"username":"viewer","password":"Viewer@123"}' \
    -o /dev/null -w '%{http_code}' "$BASE_URL/api/rbac/login")"
expect "viewer dang nhap -> 200" "200" "$VIEWER_LOGIN"

VIEWER_UI="$(curl --max-time 10 -s -b "$VIEWER_JAR" -L \
    -o "$TMP/viewer-ui.html" -w '%{http_code}' "$BASE_URL/ui")"
expect "viewer mo /ui -> 200" "200" "$VIEWER_UI"

VIEWER_STATE="$(curl --max-time 10 -s -b "$VIEWER_JAR" -o /dev/null \
    -w '%{http_code}' "$BASE_URL/api/state")"
expect "viewer xem thong so -> 200" "200" "$VIEWER_STATE"

VIEWER_CONTROL="$(curl --max-time 10 -s -b "$VIEWER_JAR" \
    -H 'Content-Type: application/json' -H "Origin: $BASE_URL" \
    -H 'X-Requested-With: fetch' -d '{"action":"ON"}' \
    -o "$TMP/viewer-control.json" -w '%{http_code}' "$BASE_URL/api/device/control")"
expect "viewer dieu khien -> 403" "403" "$VIEWER_CONTROL"
if grep -q '"requiredPermission":"device:control"' "$TMP/viewer-control.json"; then
    PASS=$((PASS + 1))
    log "PASS  403 neu ro requiredPermission=device:control"
else
    FAIL=$((FAIL + 1))
    log "FAIL  403 thieu requiredPermission"
fi

LAMP_AFTER_VIEWER="$(curl --max-time 10 -s -b "$VIEWER_JAR" "$BASE_URL/api/state" \
    | sed -n 's/.*"lamp":"\([A-Z]*\)".*/\1/p')"
log "INFO  Trang thai den sau khi viewer thu dieu khien: $LAMP_AFTER_VIEWER"

log ""
log "[3] Tai khoan admin"
ADMIN_LOGIN="$(curl --max-time 10 -s -c "$ADMIN_JAR" \
    -H 'Content-Type: application/json' -H "Origin: $BASE_URL" \
    -d '{"username":"admin","password":"Admin@123"}' \
    -o /dev/null -w '%{http_code}' "$BASE_URL/api/rbac/login")"
expect "admin dang nhap -> 200" "200" "$ADMIN_LOGIN"

ADMIN_USERS="$(curl --max-time 10 -s -b "$ADMIN_JAR" -o "$TMP/users.json" \
    -w '%{http_code}' "$BASE_URL/api/rbac/users")"
expect "admin doc danh sach tai khoan -> 200" "200" "$ADMIN_USERS"
if grep -q 'password_hash\|scrypt\$' "$TMP/users.json"; then
    FAIL=$((FAIL + 1))
    log "FAIL  API lo password hash"
else
    PASS=$((PASS + 1))
    log "PASS  API khong lo password hash"
fi

ADMIN_CONTROL="$(curl --max-time 10 -s -b "$ADMIN_JAR" \
    -H 'Content-Type: application/json' -H "Origin: $BASE_URL" \
    -H 'X-Requested-With: fetch' -d '{"action":"ON"}' \
    -o "$TMP/admin-control.json" -w '%{http_code}' "$BASE_URL/api/device/control")"
expect "admin dieu khien -> 200" "200" "$ADMIN_CONTROL"
if grep -q '"lamp":"ON"' "$TMP/admin-control.json"; then
    PASS=$((PASS + 1))
    log "PASS  Trang thai den = ON"
else
    FAIL=$((FAIL + 1))
    log "FAIL  Trang thai den khong phai ON"
fi

log ""
log "=========================================================================="
log "KET QUA SMOKE: $PASS PASS, $FAIL FAIL"
log "=========================================================================="
if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
