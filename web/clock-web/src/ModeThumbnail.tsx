import { Col } from "react-bootstrap";

type ModeThumbnailProps = {
  mode: Mode;
  colors?: ColorMap;
  onClick: (index:number) => void
};

const rgbToHex = (color: Color) => {
  return (
    "#" +
    ((1 << 24) + (color.r << 16) + (color.g << 8) + color.b)
      .toString(16)
      .slice(1)
  );
};
/*
const colorIndexToHex = (colorIndex: number, colors: ColorMap | undefined) => {
  const color = colors?.[colorIndex];
  if (color) {
    return rgbToHex(color);
  }

  return "#000000";
};
*/
const colorNameToHex = (colorName: string, colors: ColorMap | undefined) => {
  for (const index in colors) {
    const color = colors[index];
    if (color.name == colorName) {
      return rgbToHex(color);
    }
  }

  return "#000000";
};

function ModeThumbnail({ mode, colors, onClick }: ModeThumbnailProps) {
  if (mode.type == "EMPTY") {
    return <></>;
  }
  if (mode.type == "OFF") {
    return (
        <button style={{ backgroundColor: "lightgray", height: "4em", width:"8em" }} className="btn m-2 overflow-hidden" onClick={_ =>onClick(mode.index)}>Off</button>
    );
  }

  return (
        <button style={{ backgroundColor: colorNameToHex(mode.color, colors), height: "4em", width:"8em"  }} className="btn m-2 overflow-hidden" onClick={_ =>onClick(mode.index)}>
          {mode.name}<br/> ({mode.type})
        </button>
  );
}

export default ModeThumbnail;
