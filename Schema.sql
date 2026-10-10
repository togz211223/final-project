-- 1. Create Users Table
CREATE TABLE Users (
    user_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role_id INTEGER NOT NULL CHECK (role_id IN (1, 2)), -- 1 = Admin, 2 = Member
    unpaid_fines NUMERIC(10, 2) DEFAULT 0.00
);

-- 2. Create Assets Table
CREATE TABLE Assets (
    asset_id SERIAL PRIMARY KEY,
    title VARCHAR(200) NOT NULL,
    author VARCHAR(150),
    type VARCHAR(50) NOT NULL, -- 'Book', 'EBook', 'Laptop'
    isbn VARCHAR(50),
    serial_number VARCHAR(100),
    category VARCHAR(100),
    quantity INTEGER NOT NULL DEFAULT 1,
    available INTEGER NOT NULL DEFAULT 1
);

-- 3. Create Borrow Records Table (With Foreign Keys)
CREATE TABLE Borrow_Records (
    loan_id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES Users(user_id) ON DELETE CASCADE,
    asset_id INTEGER REFERENCES Assets(asset_id) ON DELETE CASCADE,
    borrow_date DATE NOT NULL DEFAULT CURRENT_DATE,
    due_date DATE NOT NULL,
    return_date DATE, -- NULL means currently borrowed
    fine_assessed NUMERIC(10, 2) DEFAULT 0.00
);

-- 4. Create Waitlist Queue Table (For FIFO algorithms)
CREATE TABLE Waitlist (
    waitlist_id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES Users(user_id) ON DELETE CASCADE,
    asset_id INTEGER REFERENCES Assets(asset_id) ON DELETE CASCADE,
    request_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 5. INSERT DUMMY ADMIN & MEMBER TO TEST WITH
INSERT INTO Users (name, email, password_hash, role_id) 
VALUES ('System Admin', 'admin@lib.com', 'scrypt_hashed_password_stub', 1);

INSERT INTO Users (name, email, password_hash, role_id) 
VALUES ('John Member', 'john@member.com', 'scrypt_hashed_password_stub', 2);