import { BaseMode, Configs } from "../types";

import { ColorChooser } from "../ColorChooser";
import { Form } from "react-bootstrap";

export type ModeBaseEditProps<M extends BaseMode> = {
  mode: M;
  configs: Configs;
  colorLabel?:string;
  onChange: (mode: M) => void;
};

export function ModeBaseEdit<M extends BaseMode>({ mode, configs, colorLabel='Farbe',onChange }: ModeBaseEditProps<M>) {
  return <>
    <Form.Group className="mb-1" controlId="formBrightness">
      <Form.Label>Helligkeit</Form.Label>
      <Form.Range value={mode.brightness} min="1" max="255" onChange={e => {
        onChange({ ...mode, brightness: e.currentTarget.value });
      }} />
    </Form.Group>
    <Form.Group className="mb-1" controlId="formColor">
      <Form.Label>{colorLabel}</Form.Label>
      <ColorChooser value={mode.color} colors={configs.colors} onChange={c => onChange({ ...mode, color: c })} />
    </Form.Group>
  </>;
}

