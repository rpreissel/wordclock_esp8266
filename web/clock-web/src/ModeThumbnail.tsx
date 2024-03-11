import { Col } from "react-bootstrap";

type ModeThumbnailProps = {
  mode: Mode;
  colors?: ColorMap;
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

function ModeThumbnail({ mode, colors }: ModeThumbnailProps) {
  if (mode.type == "EMPTY") {
    return <></>;
  }
  if (mode.type == "OFF") {
    return (
      <Col xs={4} md={2}>
        <div className="text-center mb-2">Off</div>
      </Col>
    );
  }

  return (
    <Col xs={4} md={2}>
      <div style={{ backgroundColor: colorNameToHex(mode.color, colors) }} className="text-center mb-2 p-2 rounded">
        {mode.name}<br/> ({mode.type})
      </div>
    </Col>
  );
}

export default ModeThumbnail;
