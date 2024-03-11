import { useEffect, useState } from "react";
import ModeThumbnail from "./ModeThumbnail.tsx";
import { Container, Row } from "react-bootstrap";

function App() {
  const [config, setConfig] = useState<Configs | undefined>();
  const [modes, setModes] = useState<Modes | undefined>();

  useEffect(() => {
    fetch("./api/configs", {
      method: "GET",
    })
      .then((response) => response.json())
      .then((data) => {
        setConfig(data);
        console.log(data);
      })
      .catch((error) => console.log(error));
  }, []);
  useEffect(() => {
    fetch("./api/modes", {
      method: "GET",
    })
      .then((response) => response.json())
      .then((data) => {
        setModes(data);
        console.log(data);
      })
      .catch((error) => console.log(error));
  }, []);

  if (!modes || !config) {
    return <div>Loading</div>;
  }

  return (
    <Container fluid>
      <h1>Vite + React</h1>
      <Row>
       
          <ModeThumbnail key={-1} mode={{ type: "OFF", index: -1 }} />
        {modes.modes.map((mode) => (
         
            <ModeThumbnail key={mode.index} mode={mode} colors={config.colors}/>
         
        ))}
      </Row>
    </Container>
  );
}

export default App;
