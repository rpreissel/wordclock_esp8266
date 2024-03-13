import { useModes } from "./model.ts";
import ModeThumbnail from "./ModeThumbnail.tsx";
import { Container, Row } from "react-bootstrap";

function App() {
  const [model] = useModes();
  

  

  if (!model) {
    return <div>Loading</div>;
  }

  return (
    <Container fluid>
      <h1>Vite + React</h1>
      <div className="d-flex justify-content-center">
        <div style={{width: "fit-content", margin: "auto"}}>
        
            <ModeThumbnail key={-1} mode={{ type: "OFF", index: -1 }} onClick={index => model.current = index}/>
          {model.modes.map((mode) => (
          
              <ModeThumbnail key={mode.index} mode={mode} colors={model.configs.colors} onClick={index => model.current = index}/>
          
          ))}
        </div>
      </div>
    </Container>
  );
}

export default App;
