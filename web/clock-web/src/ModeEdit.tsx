import { BaseMode, Configs, Mode } from "./types";
import { Button, Form } from "react-bootstrap";

import { ColorChooser } from "./Colors";
import { useState } from "react";

type ModeEditProps = {
  mode: Mode;
  configs: Configs;
  onSave: (mode: Mode) => void;
};

type ModeBaseEditProps<M extends BaseMode> = {
  mode: M;
  configs: Configs;
  onChange: (mode: M) => void;
};

function ModeBaseEdit<M extends BaseMode>({ mode, configs, onChange }: ModeBaseEditProps<M>) {
  return <>    <Form.Group className="mb-3" controlId="formName">
      <Form.Label>Name</Form.Label>
      <Form.Control type="text" value={mode.name} onChange={e => {
        onChange({ ...mode, name: e.currentTarget.value });
      }} />
    </Form.Group>
    <Form.Group className="mb-3" controlId="formBrightness">
      <Form.Label>Brightness</Form.Label>
      <Form.Range value={mode.brightness} min="1" max="255" onChange={e => {
        onChange({ ...mode, brightness: e.currentTarget.value });
      }} />
    </Form.Group>
    <Form.Group className="mb-3" controlId="formColor">
      <Form.Label>Color</Form.Label>
      <ColorChooser value={mode.color} colors={configs.colors} onChange={c =>
        onChange({ ...mode, color: c })
      } />
    </Form.Group>
    <Button variant="primary" type="submit" className="me-2">
      Submit
    </Button>
    <Button variant="secondary" type="reset">
      Reset
    </Button>
  </>;
}

function ModeEdit({ mode, onSave, ...props }: ModeEditProps) {
  const [state, setState] = useState(mode);
  if (state.type == "EMPTY") {
    return <></>;
  }
  let formContent = <></>;
  if (state.type == "DIGICLOCK") {
    formContent = <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)}/>;
  }
  if (state.type == "WORDCLOCK") {
    formContent = <ModeBaseEdit mode={state} {...props} onChange={mode => setState(mode)}/>;
  }
  return <Form onSubmit={e => {
    e.preventDefault();
    onSave(state);
  }} onReset={e => {
    e.preventDefault();
    setState(mode);
  }}>  {formContent}
  </Form>

}

export default ModeEdit;
