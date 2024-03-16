import { Button, Form } from "react-bootstrap";

import { colorNameToHex } from "./colors";
import { useState } from "react";

type ModeEditProps = {
  mode: Mode;
  configs: Configs;
  onSave: (mode: Mode) => void;
};

type ModeBaseEditProps<M extends BaseMode> = {
  mode: M;
  configs: Configs;
  onSave: (mode: M) => void;
};

function ModeBaseEdit<M extends BaseMode>({ mode, configs, onSave }: ModeBaseEditProps<M>) {
  const [state, setState] = useState(mode);
  console.log("a:" + state.name);
  return <Form onSubmit={e => {
    e.preventDefault();
    onSave(state);
  }} onReset={e => {
    e.preventDefault();
    setState(mode);
  }}>
    <Form.Group className="mb-3" controlId="formName">
      <Form.Label>Name</Form.Label>
      <Form.Control type="text" value={state.name} onChange={e => {
        setState({ ...state, name: e.currentTarget.value });
      }} />
    </Form.Group>
    <Form.Group className="mb-3" controlId="formBrightness">
      <Form.Label>Brightness</Form.Label>
      <Form.Range value={state.brightness} min="1" max="255" onChange={e => {
        setState({ ...state, brightness: e.currentTarget.value });
      }} />
    </Form.Group>
    <Form.Group className="mb-3" controlId="formColor">
      <Form.Label>Color</Form.Label>
      <Form.Select value={state.color} onChange={e => {
        setState({ ...state, color: e.currentTarget.value });
      }}>
        {Object.keys(configs.colors).map((colorName, i) => (
          i ?
          <option style= {{background:colorNameToHex(configs.colors[colorName].name, configs.colors)}} value={configs.colors[colorName].name} key={i}>
            {configs.colors[colorName].name}
          </option> : <></>
          
        ))}

      </Form.Select>
    </Form.Group>
    <Button variant="primary" type="submit" className="me-2">
      Submit
    </Button>
    <Button variant="secondary" type="reset">
      Reset
    </Button>
  </Form>;
}

function ModeEdit({ mode, ...props }: ModeEditProps) {
  if (mode.type == "EMPTY") {
    return <></>;
  }
  if (mode.type == "DIGICLOCK") {
    return <ModeBaseEdit mode={mode} {...props} />;
  }

  return <></>;
}

export default ModeEdit;
