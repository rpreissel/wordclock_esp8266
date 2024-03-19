import { Configs, PictureMode } from "../types";

import { ColorChooser } from "../ColorChooser";
import { Form } from "react-bootstrap";
import { colorNameToHex } from "../colors";
import { range } from "../range";
import { useState } from "react";

type ColorRgbMap = { [k: string]: string };

type PictureRowProps = {
  colors: ColorRgbMap;
  text: string;
  onClick: (col:number) => void
};

const PictureRow = ({ text, colors, onClick }: PictureRowProps) => {
  return <>
    {range(0, 11).map((index) => {
      const color = index < text.length && text[index] != ' ' ? colors[text[index]] : undefined;
      return <div key={index} className="piccell" style={{ backgroundColor: color }} onClick={() => onClick(index)}>
        {index < text.length ? text[index] : ' '}
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
    1: colorNameToHex(mode.color, configs.colors),
    2: colorNameToHex(mode.color1, configs.colors),
    3: colorNameToHex(mode.color2, configs.colors)
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
      <Form.Check
        inline
        label="Color1"
        name="color"
        type="radio"
        checked={color === "1"}
        onChange={() => setColor("1")}
      />
      <Form.Check
        inline
        label="Color2"
        name="color"
        type="radio"
        checked={color === "2"}
        onChange={() => setColor("2")}
      />
      <Form.Check
        inline
        label="Color3"
        name="color"
        type="radio"
        checked={color === "3"}
        onChange={() => setColor("3")}
      />
    </div>
    <div className="picgrid">
      {range(0, 11).map((index) => {
        const rowText = mode.pixels[index.toString(16)];
        return <PictureRow key={index} text={rowText} colors={colors} onClick={col =>{
          console.log("onClick " + index + "/" +col);
          const newColor = rowText[col]!==color.toString() ? color.toString() : ' ';
          const newRowText= rowText.substring(0,col) + newColor + rowText.substring(col+1);
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
