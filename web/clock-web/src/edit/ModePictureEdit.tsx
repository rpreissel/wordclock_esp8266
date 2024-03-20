import { ButtonGroup, Form, ToggleButton } from "react-bootstrap";
import { Configs, PictureMode } from "../types";

import { ColorChooser } from "../ColorChooser";
import { colorNameToHex } from "../colors";
import { range } from "../range";
import { useState } from "react";

type ColorRgbMap = { [k: string]: string };

type PictureRowProps = {
  colors: ColorRgbMap;
  colorString: string;
  textString: string;
  onClick: (col:number) => void
};

const PictureRow = ({ colorString: colorString,textString, colors, onClick }: PictureRowProps) => {
  return <>
    {range(0, 11).map((index) => {
      const color = colors[index < colorString.length ? colorString[index] : ' '];
      return <div key={index} className="piccell" style={{ color: color }} onClick={() => onClick(index)}>
        {index < textString.length ? textString[index] : ' '}
      </div>;
    })
    }
  </>
}

type ModePictureEditProps = {
  mode: PictureMode;
  configs: Configs;
  onChange: (mode: PictureMode) => void;
};

export function ModePictureEdit({ mode, configs, onChange }: ModePictureEditProps) {
  const [color, setColor] = useState("1");
  const colors: ColorRgbMap = {
    ' ': "gray",
    1: colorNameToHex(mode.color, configs.colors),
    2: mode.color1==="off" ? "gray" : colorNameToHex(mode.color1, configs.colors),
    3: mode.color2==="off" ? "gray" : colorNameToHex(mode.color2, configs.colors)
  };
  return <>
    <Form.Group className="mb-1" controlId="formColor1">
      <Form.Label>Color2</Form.Label>
      <ColorChooser value={mode.color1} colors={configs.colors} showOffColor onChange={c => onChange({ ...mode, color1: c })} />
    </Form.Group>
    <Form.Group className="mb-1" controlId="formColor2">
      <Form.Label>Color3</Form.Label>
      <ColorChooser value={mode.color2} colors={configs.colors} showOffColor onChange={c => onChange({ ...mode, color2: c })} />
    </Form.Group>
    <hr />
    <div className="mb-1">
    <ButtonGroup>
      <ToggleButton
        id="color1"
        type="radio"
        value={"1"}
        checked={color === "1"}
        variant="secondary"
        onChange={() => setColor("1")}> Color1 </ToggleButton>
      <ToggleButton
        id="color2"
        type="radio"
        value={"2"}
        checked={color === "2"}
        variant="secondary"
        onChange={() => setColor("2")}> Color2 </ToggleButton>
      <ToggleButton
        id="color3"
        type="radio"
        value={"3"}
        variant="secondary"
        checked={color === "3"}
        onChange={() => setColor("3")}> Color3 </ToggleButton>
        </ButtonGroup>
    </div>
    <div className="picgrid mb-2">
      {range(0, 11).map((index) => {
        const rowColors = mode.pixels[index.toString(16)];
        return <PictureRow key={index} colorString={rowColors} textString={configs.leds[index.toString(16)]} colors={colors} onClick={col =>{
          console.log("onClick " + index + "/" +col);
          const newColor = rowColors[col]!==color.toString() ? color.toString() : ' ';
          const newRowText= rowColors.substring(0,col) + newColor + rowColors.substring(col+1);
          const newPixels = {...mode.pixels };
          newPixels[index.toString(16)] = newRowText;
          console.log(newPixels);
          onChange({ ...mode, pixels: newPixels })
        }}/>
      })
      }
    </div>
  </>;
}
