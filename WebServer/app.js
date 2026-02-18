'use strict';

var WebSocket = require('ws');

// ルーム管理用のオブジェクト
const activeRooms = new Map();

//websocket server
const wss = new WebSocket.Server({ port: 9002 });
wss.on('connection', function (ws) {
	console.log('Client connected');

	// クライアントからのメッセージを受信
	ws.on('message', function (message) {
		try {
			const messageData = JSON.parse(message);
			console.log('Received: %s', messageData);

			// ルーム作成リクエストの処理
			if (messageData.request === "createRoom") {
				// ルーム作成処理
				const roomId = Math.random().toString(36).substring(2, 10); // ランダムなルームID生成

				ws.isHost = true; // クライアントをホストとしてマーク
				ws.roomId = roomId; // クライアントにルームIDを保存

				// ルームをアクティブなルームに追加
				activeRooms.set(roomId, {host: ws, guests:[]});

				ws.send(JSON.stringify({ type: 'roomCreated', roomId: roomId }));
				console.log(`Room ${roomId} created by host.`);
			}

			// ルーム参加リクエストの処理
			if (messageData.request === "joinRoom") {
				// Mapの最初のキーを取得
				const availableRoomId = activeRooms.keys().next().value;

				if (availableRoomId) {
					const room = activeRooms.get(availableRoomId);

					// 現在使用中の番号を取得
					const usedNumbers = room.guests.map(guest => guest.guestNumber);
					// 1から順に確認して最小の数字を探す
					let guestNumber = 1;
					while (usedNumbers.includes(guestNumber)) {
						guestNumber++;
					}

					ws.roomId = availableRoomId; // クライアントにルームIDを保存
					ws.isHost = false; // クライアントをホストではないとしてマーク
					ws.isReady = false; // クライアントの準備状態を初期化
					ws.guestNumber = guestNumber; // クライアントに参加者番号を保存

					room.guests.push(ws);

					var isReadyMember = 0x1;
					// 参加者の準備状態をビットフラグで管理
					room.guests.forEach(guest => {
						// ホストは常に準備完了とみなす
						if (guest.isReady) {
							isReadyMember |= (1 << guest.guestNumber); // 準備完了の参加者のビットを立てる
						}
					});

					ws.send(JSON.stringify({ type: 'roomJoined', roomId: availableRoomId, guestNumber: guestNumber, memberCount: room.guests.length + 1, readyMember: isReadyMember }));
					console.log(`Client joined room ${availableRoomId}.`);

					// ホストに参加者がいることを通知
					if (room.host.readyState === WebSocket.OPEN) {
						const joinMessage = {
							type: 'guestJoined',
							playerName: messageData.playerName,

							guestNumber: guestNumber
						};
						room.host.send(JSON.stringify(joinMessage));
					}

					// 他の参加者にも新しい参加者がいることを通知
					room.guests.forEach(guest => {
						if (guest !== ws && guest.readyState === WebSocket.OPEN) {
							const joinMessage = {
								type: 'newGuestJoined',
								playerName: messageData.playerName,
								guestNumber: guestNumber
							};
							guest.send(JSON.stringify(joinMessage));
						}
					});
				} else {
					// 利用可能なルームがない場合の処理
					ws.send(JSON.stringify({ type: 'error', message: 'No available rooms to join.' }));
				}
			}

			// 参加者準備完了通知の処理
			if (messageData.type === 'guestReady') {
                ws.isReady = messageData.ready; // クライアントの準備状態を更新

				const room = activeRooms.get(ws.roomId);
				// ホストに参加者の準備完了を通知
				room.host.send(JSON.stringify({ type: 'guestReady', ready: ws.isReady, guestNumber: ws.guestNumber }));

				// 自信を含め他の参加者にも通知
				room.guests.forEach(guest => {
					if (guest.readyState === WebSocket.OPEN) {
						guest.send(JSON.stringify({ type: 'guestReady', ready: ws.isReady, guestNumber: ws.guestNumber }));
					}
				});

				console.log(`Guest ${ws.guestNumber} is ready in room ${ws.roomId}.`);
			}

			// ホスト開始通知の処理
			if (messageData.type === 'hostStart') {
				const room = activeRooms.get(ws.roomId);
				room.host.send(message);
				room.guests.forEach(guest => {
					if (guest.readyState === WebSocket.OPEN) {
						guest.send(JSON.stringify({ type: 'hostStart' }));
					}
                });
			}

			// ユーザーの更新データの処理
			if (messageData.type === 'playerUpdate') {
				const room = activeRooms.get(ws.roomId);
				if (room.host != ws) {
					room.host.send(message);
				}
				room.guests.forEach(guest => {
					if (guest != ws && guest.readyState === WebSocket.OPEN) {
						guest.send(message); // 参加者に更新データを転送
					}
				});
			}

			// ゲスト側のシーン遷移完了
			if (messageData.type === 'guestCompletedSceneChange') {
				const room = activeRooms.get(ws.roomId);
				room.host.send(JSON.stringify({ type: 'guestCompletedSceneChange', userId: messageData.userId }));
			}

			// レース開始通知の処理
			if (messageData.type === 'startRace') {
				const room = activeRooms.get(ws.roomId);
				room.host.send(message);
				room.guests.forEach(guest => {
					if (guest.readyState === WebSocket.OPEN) {
						guest.send(message);
					}
				});
			}

			// ゲスト退出
			if (messageData.type === 'guestLeave') {
				const room = activeRooms.get(ws.roomId);
				if (room && !ws.isHost) {
					// 参加者をルームから削除
					room.guests = room.guests.filter(guest => guest !== ws);

					// ホストに参加者が退出したことを通知
					if (room.host.readyState === WebSocket.OPEN) {
						room.host.send(JSON.stringify({ type: 'guestLeft', guestNumber: messageData.guestNumber }));
					}
				}

				// 他の参加者にも通知
				room.guests.forEach(guest => {
					if (guest !== ws && guest.readyState === WebSocket.OPEN) {
						guest.send(JSON.stringify({ type: 'guestLeft', guestNumber: messageData.guestNumber }));
					}
				});

				console.log(`Guest ${messageData.guestNumber} left room ${ws.roomId}.`);
				ws.roomId = null; // クライアントのルームIDをクリア
				ws.guestNumber = null; // クライアントの参加者番号をクリア
			}

			// ホスト退出
			if (messageData.type === 'closeRoom') {
				// このクライアントがホストであることを確認
				if (!ws.isHost) {
					ws.send(JSON.stringify({ type: 'error', message: 'Only the host can close the room.' }));
					return;
                }

				// ルームをアクティブなルームから削除
				activeRooms.delete(ws.roomId);
				console.log(`Room ${ws.roomId} closed by host.`);

				// 参加者にルームが閉じられたことを通知
				wss.clients.forEach(client => {
					// 参加者が同じルームにいる場合
					if (client.roomId === ws.roomId && client.readyState === WebSocket.OPEN) {
						// クライアントのルームIDをクリア
						client.roomId = null;

						// ルーム閉鎖メッセージを送信
						client.send(JSON.stringify({ type: 'roomClosed' }));
					}
				});

				ws.roomId = null; // クライアントのルームIDをクリア
				ws.isHost = false; // クライアントのホストフラグをクリア
				ws.guestNumber = null; // クライアントの参加者番号をクリア

			}


		} catch (e) {
			console.error('Error parsing message:', e);
		}
	});
	ws.on('close', function () {
		console.log('Client disconnected');

		// ホストが切断した場合、ルームを閉じる
		if (ws.isHost && ws.roomId) {
			activeRooms.delete(ws.roomId);
			console.log(`Room ${ws.roomId} closed as host disconnected.`);

			// 参加者にルームが閉じられたことを通知
			wss.clients.forEach(client => {
				// 参加者が同じルームにいる場合
				if (client.roomId === ws.roomId && client.readyState === WebSocket.OPEN) {
					// ルーム閉鎖メッセージを送信
					client.send(JSON.stringify({ type: 'roomClosed' }));
				}
			});
		} else if (ws.roomId) {
			// 参加者が切断した場合、ホストに通知
			const room = activeRooms.get(ws.roomId);
			if (room) {
				room.guests = room.guests.filter(guest => guest !== ws); // 参加者をルームから削除

				// ホストに参加者が切断したことを通知
				if (room.host.readyState === WebSocket.OPEN) {
					room.host.send(JSON.stringify({ type: 'guestLeft', guestNumber: ws.guestNumber }));
				}
				// 他の参加者にも通知
				room.guests.forEach(guest => {
					if (guest !== ws && guest.readyState === WebSocket.OPEN) {
						guest.send(JSON.stringify({ type: 'guestLeft', guestNumber: ws.guestNumber }));
					}
				});

			}

		}

	});

	//初期メッセージを送信
	ws.send(JSON.stringify({ type: 'welcome', message: 'Welcome to the WebSocket server!' }));
});

console.log('Hello world');