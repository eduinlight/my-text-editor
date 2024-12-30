#include <iostream>
#include <map>
#include <string>

class PieceTable {
public:
  struct Piece {
    bool isOriginal; // true = pertenece al buffer original, false = buffer de
                     // adiciones
    size_t offset;   // Offset en el buffer
    size_t length;   // Longitud de la pieza
  };

private:
  std::string originalBuffer;     // Buffer original
  std::string additionBuffer;     // Buffer de adiciones
  std::map<size_t, Piece> pieces; // Mapa de piezas, clave: posición en el texto

  // Ajusta las claves del mapa después de una operación
  void adjustKeys(size_t startPos, size_t delta) {
    std::map<size_t, Piece> newPieces;
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
      size_t newPos = it->first >= startPos ? it->first + delta : it->first;
      newPieces[newPos] = it->second;
    }
    pieces = std::move(newPieces);
  }

public:
  // Constructor
  PieceTable(const std::string &initialText) {
    originalBuffer = initialText;
    pieces[0] = {true, 0, originalBuffer.size()};
  }

  // Inserta texto en una posición
  void insert(size_t position, const std::string &text) {
    if (position > getLength())
      throw std::out_of_range("Posición fuera de rango");

    size_t additionOffset = additionBuffer.size();
    additionBuffer += text;

    auto it = pieces.lower_bound(position);
    if (it != pieces.begin() && (it == pieces.end() || it->first > position)) {
      --it;
    }

    size_t currentPos = it->first;
    Piece currentPiece = it->second;

    size_t pieceStart = position - currentPos;

    // Dividir la pieza afectada
    if (pieceStart > 0) {
      pieces[currentPos] = {currentPiece.isOriginal, currentPiece.offset,
                            pieceStart};
      currentPos += pieceStart;
    }

    // Insertar la nueva pieza
    pieces[currentPos] = {false, additionOffset, text.size()};
    currentPos += text.size();

    // Resto de la pieza original
    if (pieceStart < currentPiece.length) {
      pieces[currentPos] = {currentPiece.isOriginal,
                            currentPiece.offset + pieceStart,
                            currentPiece.length - pieceStart};
    }

    // Ajustar las claves
    adjustKeys(currentPos, text.size());
  }

  // Eliminar texto en un rango
  void erase(size_t position, size_t length) {
    if (position + length > getLength())
      throw std::out_of_range("Rango fuera de los límites");

    auto it = pieces.lower_bound(position);
    if (it != pieces.begin() && (it == pieces.end() || it->first > position)) {
      --it;
    }

    size_t currentPos = it->first;
    std::map<size_t, Piece> newPieces;

    while (length > 0) {
      Piece piece = it->second;
      size_t pieceStart = position - currentPos;
      size_t toRemove = std::min(length, piece.length - pieceStart);

      // Mantener la parte anterior a la eliminación
      if (pieceStart > 0) {
        newPieces[currentPos] = {piece.isOriginal, piece.offset, pieceStart};
        currentPos += pieceStart;
      }

      // Saltar la parte eliminada
      currentPos += toRemove;
      length -= toRemove;

      // Mantener la parte posterior a la eliminación
      if (pieceStart + toRemove < piece.length) {
        newPieces[currentPos] = {piece.isOriginal,
                                 piece.offset + pieceStart + toRemove,
                                 piece.length - pieceStart - toRemove};
        currentPos += piece.length - pieceStart - toRemove;
      }

      ++it;
    }

    pieces = std::move(newPieces);
  }

  // Obtiene el texto completo
  std::string getText() const {
    std::string result;

    for (const auto &[start, piece] : pieces) {
      const auto &buffer = piece.isOriginal ? originalBuffer : additionBuffer;
      result += buffer.substr(piece.offset, piece.length);
    }

    return result;
  }

  // Obtiene la longitud total del texto
  size_t getLength() const {
    size_t length = 0;
    for (const auto &[_, piece] : pieces) {
      length += piece.length;
    }
    return length;
  }
};

int main() {
  PieceTable editor("Hola, mundo!");
  std::cout << "Texto inicial: " << editor.getText() << "\n";

  editor.insert(5, "querido ");
  std::cout << "Después de insertar: " << editor.getText() << "\n";

  editor.erase(5, 8);
  std::cout << "Después de borrar: " << editor.getText() << "\n";

  return 0;
}
