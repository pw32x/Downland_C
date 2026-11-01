import socket
import threading
import sys
import time

if len(sys.argv) != 3:
    print(f"Usage: py {sys.argv[0]} <ip> <port>")
    sys.exit(1)

HOST = sys.argv[1]
PORT = int(sys.argv[2])

PLAYER_DATA_SIZE = 5
CLIENT_TIMEOUT = 5.0
UPDATE_INTERVAL = 1.0 / 20.0
MAX_PLAYERS = 2

clients = {}
clients_lock = threading.Lock()


def send_to_client(conn, data):
    """Send data safely, serializing writes to each socket."""
    with clients_lock:
        client = clients.get(conn)

    if client is None:
        return False

    try:
        with client["send_lock"]:
            conn.sendall(data)
        return True
    except OSError:
        return False


def handle_client(conn, addr, player_number):
    print(f"Player {player_number} connected: {addr}")

    conn.settimeout(0.5)
    buffer = bytearray()
    last_received = time.monotonic()

    with clients_lock:
        clients[conn] = {
            "addr": addr,
            "player_number": player_number,
            "send_lock": threading.Lock(),
            "latest_data": None
        }

    try:
        while True:
            try:
                data = conn.recv(1024)
            except socket.timeout:
                data = None

            if data == b"":
                break

            if data:
                buffer.extend(data)
                last_received = time.monotonic()

                # TCP is a byte stream. Assemble complete five-byte records.
                while len(buffer) >= PLAYER_DATA_SIZE:
                    packet = bytes(buffer[:PLAYER_DATA_SIZE])
                    del buffer[:PLAYER_DATA_SIZE]

                    player_id = packet[0]
                    room_number = packet[1]
                    x = packet[2]
                    y = packet[3]
                    frame = packet[4]

                    print(
                        f"[Player {player_number}] "
                        f"Player={player_id}, "
                        f"Room={room_number}, "
                        f"X={x}, Y={y}, Frame={frame}"
                    )

                    # Replace the pending update for the other player.
                    with clients_lock:
                        for other_conn, other_client in clients.items():
                            if other_conn != conn:
                                other_client["latest_data"] = packet

            if time.monotonic() - last_received > CLIENT_TIMEOUT:
                print(f"Player {player_number} timed out.")
                break

    except OSError as e:
        print(f"[Player {player_number}] Connection error: {e}")

    finally:
        with clients_lock:
            clients.pop(conn, None)

        try:
            conn.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

        conn.close()
        print(f"Player {player_number} disconnected: {addr}")


def send_player_updates():
    """Send each player the other player's latest update at 20 FPS."""
    while True:
        frame_start = time.monotonic()

        with clients_lock:
            updates = []

            for conn, client in clients.items():
                packet = client["latest_data"]

                if packet is not None:
                    updates.append((conn, client, packet))
                    client["latest_data"] = None

        for conn, client, packet in updates:
            if not send_to_client(conn, packet):
                print(
                    f"Failed to send player data to {client['addr']}."
                )

        elapsed = time.monotonic() - frame_start
        time.sleep(max(0.0, UPDATE_INTERVAL - elapsed))


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((HOST, PORT))
        server.listen()

        print(f"Listening on {HOST}:{PORT}...")
        print(f"Maximum players: {MAX_PLAYERS}")
        print("Player update rate: 20 FPS")
        print(f"Client timeout: {CLIENT_TIMEOUT} seconds")
        print("Waiting for players...")

        threading.Thread(
            target=send_player_updates,
            daemon=True
        ).start()

        next_player_number = 1

        try:
            while True:
                conn, addr = server.accept()

                with clients_lock:
                    if len(clients) >= MAX_PLAYERS:
                        reject = True
                        player_number = None
                    else:
                        reject = False
                        player_number = next_player_number
                        next_player_number += 1

                if reject:
                    print(f"Rejected extra connection: {addr}")
                    conn.close()
                    continue

                threading.Thread(
                    target=handle_client,
                    args=(conn, addr, player_number),
                    daemon=True
                ).start()

        except KeyboardInterrupt:
            print("\nServer shutting down.")


if __name__ == "__main__":
    main()
