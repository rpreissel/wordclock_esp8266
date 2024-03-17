import { Color, ColorMap } from "./types";

export const rgbToHex = (color: Color) => {
  return (
    "#" +
    ((1 << 24) + (color.r << 16) + (color.g << 8) + color.b)
      .toString(16)
      .slice(1)
  );
};

export const colorIndexToHex = (
  colorIndex: number,
  colors: ColorMap | undefined
) => {
  const color = colors?.[colorIndex];
  if (color) {
    return rgbToHex(color);
  }

  return "#000000";
};

export const colorNameToHex = (
  colorName: string,
  colors: ColorMap | undefined
) => {
  for (const index in colors) {
    const color = colors[index];
    if (color.name == colorName) {
      return rgbToHex(color);
    }
  }

  return "#000000";
};
