# rioki's signal library

## Getting Started

rsig provides a signal class that is simple and thread safe. All you need to do 
is create a signal, hock up a function to it and start handling events.

For the start, let's take this contrived example:

    rsig::signal<unsigned int, unsigned int> processing_signal;

    processing_signal.connect([&] (auto done, auto total) {
        auto percent = static_cast<float>(done) / static_cast<float>(total) * 100.0f;
        std::cout << "Handled " << done << " of " << total << " [" << percent << "%]" << std::endl;
    });

    for (auto i = 0u; i < items.size(); i++)
    {
        process_item(items[i]);
        processing_signal.emit(i+1, items.size());
    }

Although contrived, this example shows the basic usage pattern of rsig. You
create a signal, connect any number of observers and emit signals when ever 
necessary.

## License

    rsig - rioki's signal library
    Copyright (c) 2020 Sean Farrell

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
 