/**
 * Engine/MapEngine/Captcha.js
 *
 * Manage Captcha packets and UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import CaptchaUpload from 'UI/Components/Captcha/CaptchaUpload.js';
import CaptchaSelector from 'UI/Components/Captcha/CaptchaSelector.js';
import CaptchaAnswer from 'UI/Components/Captcha/CaptchaAnswer.js';
import CaptchaPreview from 'UI/Components/Captcha/CaptchaPreview.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import DB from 'DB/DBManager.js';

/**
 * Captcha data
 */
const captcha = {
	captchaKey: null,
	imageSize: null,
	currentOffset: 0,
	imageData: null,
	retryCount: 0,
	timeout: 0
};

/**
 * Reset captcha
 */
function resetCaptcha() {
	captcha.captchaKey = null;
	captcha.imageSize = null;
	captcha.currentOffset = 0;
	captcha.imageData = null;
	captcha.retryCount = 0;
	captcha.timeout = 0;
}

/**
 * Received upload acknowledgment
 * @param {object} pkt - PACKET.ZC.ACK_UPLOAD_MACRO_DETECTOR
 */
function onAckUpload(pkt) {
	// 0 = success
	// 1 = fail
	if (pkt.captchaFlag === 0) {
		// upload captcha in chunks of 1024 bytes
		const totalChunks = Math.ceil(CaptchaUpload.imageData.byteLength / 1024);
		let currentChunk = 0;
		const buffer = CaptchaUpload.imageData.buffer;
		let offset = 0;

		while (currentChunk < totalChunks) {
			const chunkSize = Math.min(1024, buffer.byteLength - offset);
			const chunk = buffer.slice(offset, offset + chunkSize);
			CaptchaUpload.uploadCaptcha(pkt.captchaKey, chunk);
			offset += chunkSize;
			currentChunk++;
		}

		return;
	}
	CaptchaUpload.uploadError();
}

/**
 * Received captcha upload complete
 * @param {object} pkt - PACKET.ZC.COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA
 */
function onCompleteUpload(pkt) {
	CaptchaUpload.uploadSuccess();
}

/**
 * Received apply acknowledgement
 * @param {object} pkt - PACKET.ZC.ACK_APPLY_MACRO_DETECTOR
 */
function onAckApply(pkt) {
	switch (pkt.status) {
		case 0: // MCR_MONITORING
			ChatBox.addText(DB.getMessage(2877), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 1: // MCR_NO_DATA
			ChatBox.addText(DB.getMessage(2878), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 2: // MCR_INPROGRESS
			ChatBox.addText(DB.getMessage(2879), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		default:
			ChatBox.addText('Unknown status: ' + pkt.status, ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
	}
}

/**
 * Received macro detector status
 * @param {object} pkt - PACKET.ZC.APPLY_MACRO_DETECTOR
 */
function onApply(pkt) {
	captcha.imageSize = pkt.imageSize;
	captcha.captchaKey = pkt.captchaKey;
}

/**
 * Received captcha image data
 * captcha is sent from server in chunks concatenate it until it reach the captcha size
 * @param {object} pkt - PACKET.ZC.APPLY_MACRO_DETECTOR_CAPTCHA
 */
function onApplyCaptcha(pkt) {
	// create image data if it doesn't exist
	if (captcha.imageData === null) {
		captcha.imageData = new Uint8Array(captcha.imageSize);
		captcha.currentOffset = 0;
	}

	if (captcha.currentOffset < captcha.imageSize) {
		// append image data to the captcha image data
		captcha.imageData.set(pkt.imageData, captcha.currentOffset);
		captcha.currentOffset += pkt.imageData.length;
	}

	// check if we have all the image data
	if (captcha.currentOffset === captcha.imageSize) {
		// decompress image
		decompressImage(captcha.imageData).then(imageData => {
			CaptchaAnswer.setImage(imageData);
			CaptchaAnswer.setData(3, 60); // if is not different from default the server dont send the information
			CaptchaAnswer.onSend = answer => {
				const _pkt = new PACKET.CZ.ACK_ANSWER_MACRO_DETECTOR();
				_pkt.answer = answer;
				Network.sendPacket(_pkt);
			};
			CaptchaAnswer.append();
			resetCaptcha();
		});
	}
}

/**
 * Received answer request
 * @param {object} pkt - PACKET.ZC.REQ_ANSWER_MACRO_DETECTOR
 */
function onReqAnswer(pkt) {
	CaptchaAnswer.setData(pkt.retryCount, pkt.timeout / 1000);
	CaptchaAnswer.setError(pkt.retryCount);
}

/**
 * Received close command
 * @param {object} pkt - PACKET.ZC.CLOSE_MACRO_DETECTOR
 */
function onClose(pkt) {
	switch (pkt.status) {
		case 0: // MCD_TIMEOUT
			//CaptchaAnswer.remove(); ??
			break;
		case 1: // MCD_INCORRECT
			//CaptchaAnswer.remove(); ??
			break;
		case 2: // MCD_GOOD
			CaptchaAnswer.showSuccessMessage();
			break;
	}
}

/**
 * Received preview acknowledgement
 * @param {object} pkt - PACKET.ZC.ACK_PREVIEW_MACRO_DETECTOR
 */
function onAckPreview(pkt) {
	if (pkt.captchaFlag === 0) {
		captcha.imageSize = pkt.imageSize;
		captcha.captchaKey = pkt.captchaKey;
	}
}

/**
 * Received preview image data
 * @param {object} pkt - PACKET.ZC.PREVIEW_MACRO_DETECTOR_CAPTCHA
 */
function onPreviewCaptcha(pkt) {
	// create image data if it doesn't exist
	if (captcha.imageData === null) {
		captcha.imageData = new Uint8Array(captcha.imageSize);
		captcha.currentOffset = 0;
	}

	if (captcha.currentOffset < captcha.imageSize) {
		// append image data to the captcha image data
		captcha.imageData.set(pkt.imageData, captcha.currentOffset);
		captcha.currentOffset += pkt.imageData.length;
	}

	// check if we have all the image data
	if (captcha.currentOffset === captcha.imageSize) {
		// decompress image
		decompressImage(captcha.imageData).then(imageData => {
			captcha.imageData = imageData;
			CaptchaPreview.setImage(captcha.imageData);
			CaptchaPreview.append();
			resetCaptcha();
		});
	}
}

/**
 * Received list of players in range
 * @param {object} pkt - PACKET.ZC.ACK_PLAYER_AID_IN_RANGE
 */
function onPlayerAidInRange(pkt) {
	CaptchaSelector.setPlayers(pkt.AID);
}

/**
 * Upload captcha to server
 */
function uploadCaptcha(captchaKey, imageData) {
	const pkt = new PACKET.CZ.UPLOAD_MACRO_DETECTOR_CAPTCHA();
	pkt.captchaKey = captchaKey;
	pkt.imageData = imageData;
	Network.sendPacket(pkt);
}

/**
 * Request to send captcha to server
 *
 * @param {Uint8Array} compressedBuffer
 * @param {string} answer
 */
async function requestUploadCaptcha(size, answer) {
	const pkt = new PACKET.CZ.REQ_UPLOAD_MACRO_DETECTOR();
	pkt.imageSize = size;
	pkt.answer = answer;
	Network.sendPacket(pkt);
}

/**
 * Send captcha to player
 *
 * @param {number} AID
 */
function sendCaptchaToPlayer(AID) {
	const pkt = new PACKET.CZ.REQ_APPLY_MACRO_DETECTOR();
	pkt.AID = AID;
	Network.sendPacket(pkt);
}

/**
 * Request players ids in range
 *
 * @param {number} xPos
 * @param {number} yPos
 * @param {number} RadiusRange
 */
function requestPlayersIdsInRange(xPos, yPos, RadiusRange) {
	const pkt = new PACKET.CZ.REQ_PLAYER_AID_IN_RANGE();
	pkt.xPos = xPos;
	pkt.yPos = yPos;
	pkt.RadiusRange = RadiusRange;
	Network.sendPacket(pkt);
}

/**
 * Compress image
 *
 * @param {File} data
 */
async function compressImage(data) {
	// Create compression stream (gzip)
	const compressionStream = new CompressionStream('deflate');

	// Pipe file stream → compression stream
	const compressedStream = data.stream().pipeThrough(compressionStream);

	// Read compressed data
	const reader = compressedStream.getReader();
	const chunks = [];

	// eslint-disable-next-line no-constant-condition
	while (true) {
		const { value, done } = await reader.read();
		if (done) {
			break;
		}
		chunks.push(value);
	}

	// Combine chunks into a single buffer
	const compressedBuffer = new Uint8Array(chunks.reduce((acc, c) => acc + c.length, 0));

	let offset = 0;
	for (const chunk of chunks) {
		compressedBuffer.set(chunk, offset);
		offset += chunk.length;
	}

	return compressedBuffer;
}

/**
 * Decompress image
 *
 * @param {Uint8Array} compressedBuffer
 */
async function decompressImage(gzipUint8Array) {
	// Cria um ReadableStream a partir do Uint8Array
	const compressedStream = new ReadableStream({
		start(controller) {
			controller.enqueue(gzipUint8Array);
			controller.close();
		}
	});

	// Stream de descompressão gzip
	const decompressionStream = new DecompressionStream('deflate');

	// Pipe: compressed → gzip → decompressed
	const decompressedStream = compressedStream.pipeThrough(decompressionStream);

	const reader = decompressedStream.getReader();
	const chunks = [];

	// eslint-disable-next-line no-constant-condition
	while (true) {
		const { value, done } = await reader.read();
		if (done) {
			break;
		}
		chunks.push(value);
	}

	// Concatena os chunks
	const decompressedBuffer = new Uint8Array(chunks.reduce((acc, c) => acc + c.length, 0));

	let offset = 0;
	for (const chunk of chunks) {
		decompressedBuffer.set(chunk, offset);
		offset += chunk.length;
	}

	return decompressedBuffer;
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.ACK_UPLOAD_MACRO_DETECTOR, onAckUpload);
	Network.hookPacket(PACKET.ZC.COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA, onCompleteUpload);
	Network.hookPacket(PACKET.ZC.ACK_APPLY_MACRO_DETECTOR, onAckApply);
	Network.hookPacket(PACKET.ZC.APPLY_MACRO_DETECTOR, onApply);
	Network.hookPacket(PACKET.ZC.APPLY_MACRO_DETECTOR_CAPTCHA, onApplyCaptcha);
	Network.hookPacket(PACKET.ZC.REQ_ANSWER_MACRO_DETECTOR, onReqAnswer);
	Network.hookPacket(PACKET.ZC.CLOSE_MACRO_DETECTOR, onClose);
	Network.hookPacket(PACKET.ZC.ACK_PREVIEW_MACRO_DETECTOR, onAckPreview);
	Network.hookPacket(PACKET.ZC.PREVIEW_MACRO_DETECTOR_CAPTCHA, onPreviewCaptcha);
	Network.hookPacket(PACKET.ZC.ACK_PLAYER_AID_IN_RANGE, onPlayerAidInRange);

	CaptchaUpload.requestUploadCaptcha = requestUploadCaptcha;
	CaptchaUpload.uploadCaptcha = uploadCaptcha;
	CaptchaUpload.compressImage = compressImage;

	CaptchaSelector.requestPlayersIdsInRange = requestPlayersIdsInRange;
	CaptchaSelector.sendCaptchaToPlayer = sendCaptchaToPlayer;
}
