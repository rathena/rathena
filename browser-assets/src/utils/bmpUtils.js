const { createCanvas, loadImage } = require('canvas');

/**
 * Cria uma imagem a partir de um buffer BMP.
 *
 * @param {Buffer} buffer - O buffer contendo os dados BMP.
 * @returns {Canvas} - A imagem criada a partir do BMP.
 */
function imagecreatefrombmp(buffer) {
  // Verificar se o buffer é válido
  if (!buffer || buffer.length < 54) {
    throw new Error('Invalid BMP file');
  }

  // Verificar assinatura do BMP
  const signature = buffer.toString('utf-8', 0, 2);
  if (signature !== 'BM') {
    throw new Error('Not a BMP file');
  }

  // Ler cabeçalho do BMP
  const fileSize = buffer.readUInt32LE(2);
  const dataOffset = buffer.readUInt32LE(10);
  const headerSize = buffer.readUInt32LE(14);
  const width = buffer.readInt32LE(18);
  const height = buffer.readInt32LE(22);
  const planes = buffer.readUInt16LE(26);
  const bitsPerPixel = buffer.readUInt16LE(28);

  if (planes !== 1 || bitsPerPixel !== 24) {
    throw new Error('Unsupported BMP format: Only 24-bit BMP files are supported');
  }

  // Cria um canvas e um contexto para desenhar a imagem
  const canvas = createCanvas(width, height);
  const ctx = canvas.getContext('2d');
  const imageData = ctx.createImageData(width, height);

  // Calcula o padding por linha (os dados BMP são alinhados a 4 bytes)
  const rowSize = Math.floor((bitsPerPixel * width + 31) / 32) * 4;
  const padding = rowSize - (width * 3);

  let pixelArrayOffset = dataOffset;
  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const pixelIndex = (y * width + x) * 4;
      const bufferIndex = pixelArrayOffset + (x * 3);

      // BMP armazena as cores no formato BGR
      imageData.data[pixelIndex] = buffer[bufferIndex + 2]; // Red
      imageData.data[pixelIndex + 1] = buffer[bufferIndex + 1]; // Green
      imageData.data[pixelIndex + 2] = buffer[bufferIndex]; // Blue
      imageData.data[pixelIndex + 3] = 255; // Alpha (fully opaque)
    }
    pixelArrayOffset += rowSize;
  }

  // Colocar os dados da imagem no canvas
  ctx.putImageData(imageData, 0, 0);

  return canvas;
}

module.exports = { imagecreatefrombmp };
