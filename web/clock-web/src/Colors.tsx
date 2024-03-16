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

type ColorChooserProps = {
  value: string;
  onChange: (color: string) => void;
  colors: ColorMap;
}

export const ColorChooser = ({ value, onChange, colors }: ColorChooserProps) => {
  return <div className="d-flex">
    <div style={{ width: "fit-content", margin: "auto" }}>

      {Object.keys(colors).map((colorIndex, i) => {
        const go = colors[colorIndex];
        const active = value === go.name;
        const classes = "color btn m-2 overflow-hidden " + (active ? "active-color" : "");

        return i ?
          <button key={i} disabled={active} style={{ backgroundColor: rgbToHex(go) }} className={classes} onClick={() => onChange(go.name)} />
          : <div key={i}></div>

      })}
    </div>
  </div>;
}