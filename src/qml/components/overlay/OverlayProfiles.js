// OverlayProfiles - Dot placement from Position (10% margin) and window geometry per Overlay Size.

.pragma library

const GREAT_HALF = 75;  // Great window is 150 x 150: fits the bar row (±60 px) around the mic
const GREAT_EDGE = 8;   // Great dot center to the window edge it sits on
const DIRECTIONS = { up: [0, -1], down: [0, 1], left: [-1, 0], right: [1, 0] };

// The dot center is 10% of the shorter screen side from each border; diagonals use it on both axes.
// Whole pixels, so every profile lands on the same pixel grid.
function dotCenter(position, screenWidth, screenHeight) {
  const margin = 0.1 * Math.min(screenWidth, screenHeight);
  const x = position.includes("left") ? margin : position.includes("right") ? screenWidth - margin : screenWidth / 2;
  const y = position.startsWith("up") ? margin : position.startsWith("down") ? screenHeight - margin : screenHeight / 2;
  return { x: Math.round(x), y: Math.round(y) };
}

// Great opens away from its border; diagonals follow the vertical rule.
function expansion(position) {
  if (position.startsWith("up")) return "down";
  if (position.startsWith("down")) return "up";
  return position === "left" ? "right" : "left";
}

// Window size and dot center inside the window, for an overlay size at a position.
function profile(size, position) {
  if (size === "Great") {
    const dir = expansion(position);
    const [dx, dy] = DIRECTIONS[dir];
    return {
      width: 2 * GREAT_HALF,
      height: 2 * GREAT_HALF,
      dotX: GREAT_HALF - dx * (GREAT_HALF - GREAT_EDGE),
      dotY: GREAT_HALF - dy * (GREAT_HALF - GREAT_EDGE),
      expansion: dir,
    };
  }
  if (size === "Basic") return { width: 100, height: 60, dotX: 50, dotY: 30, expansion: "" };
  return { width: 26, height: 26, dotX: 13, dotY: 13, expansion: "" };  // 26 px fits the 25 px spinner ring
}
