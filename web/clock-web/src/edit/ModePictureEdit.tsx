import { Configs, PictureMode } from "../types";

import { ColorChooser } from "../ColorChooser";
import { Form } from "react-bootstrap";

type ModePictureEditProps = {
  mode: PictureMode;
  configs: Configs;
  onChange: (mode: PictureMode) => void;
};
export function ModePictureEdit({ mode, configs, onChange }: ModePictureEditProps) {
  return <>
    <Form.Group className="mb-1" controlId="formColor1">
      <Form.Label>Color2</Form.Label>
      <ColorChooser value={mode.color1} colors={configs.colors} showOffColor onChange={c => onChange({ ...mode, color1: c })} />
    </Form.Group>
    <Form.Group className="mb-1" controlId="formColor2">
      <Form.Label>Color3</Form.Label>
      <ColorChooser value={mode.color2} colors={configs.colors} showOffColor onChange={c => onChange({ ...mode, color2: c })} />
    </Form.Group>
  </>;
}
